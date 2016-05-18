/*!
 * \file axiom_netdev_module.c
 *
 * \version     v0.5
 * \date        2016-05-03
 *
 * This file contains the implementation of the Axiom NIC kernel module.
 */
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/sched.h>

#include "evi_queue.h"

#include "axiom_nic_packets.h"
#include "axiom_netdev_module.h"
#include "axiom_netdev_user.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence SRL");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("v0.5");

/*! \brief verbose module parameter */
int verbose = 0;
module_param(verbose, int, 0);
MODULE_PARM_DESC(debug, "versbose level (0=none,...,16=all)");

/*! \brief bitmap to handle chardev minor (devnum) */
static DECLARE_BITMAP(dev_map, AXIOMNET_DEV_MAX);
/*! \brief bitmap spinloc */
static DEFINE_SPINLOCK(map_lock);

struct axiomnet_chrdev chrdev;

static int axiomnet_alloc_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);
static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);



/*********************** AxiomNet Device Driver ******************************/

/* Match table for of_platform binding */
static const struct of_device_id axiomnet_of_match[] = {
    { .compatible = AXIOMREG_COMPATIBLE, },
    {},
};

static inline void axiomnet_enable_irq(struct axiomnet_drvdata *drvdata)
{
    iowrite32(AXIOMREG_IRQ_ALL, drvdata->vregs + AXIOMREG_IO_MSKIRQ);
}

static inline void axiomnet_disable_irq(struct axiomnet_drvdata *drvdata)
{
    iowrite32(~AXIOMREG_IRQ_ALL, drvdata->vregs + AXIOMREG_IO_MSKIRQ);
}


static irqreturn_t axiomnet_irqhandler(int irq, void *dev_id)
{
    struct axiomnet_drvdata *drvdata = dev_id;
    irqreturn_t serviced = IRQ_NONE;
    uint32_t irq_pending;

    DPRINTF("start");
    irq_pending = ioread32(drvdata->vregs + AXIOMREG_IO_PNDIRQ);

    if (irq_pending & AXIOMREG_IRQ_SMALL_RX) {
        wake_up(&drvdata->small_rx_ring.wait_queue);
    }

    if (irq_pending & AXIOMREG_IRQ_SMALL_TX) {
        wake_up(&drvdata->small_tx_ring.wait_queue);
    }

    iowrite32(irq_pending, drvdata->vregs + AXIOMREG_IO_ACKIRQ);
    serviced = IRQ_HANDLED;
    DPRINTF("end");

    return serviced;
}

inline static unsigned int axiomnet_small_tx_avail(struct axiomnet_hw_ring *ring)
{
    /*TODO: try to minimize the register read */
    return axiom_hw_small_tx_avail(ring->drvdata->dev_api);
}

inline static int axiomnet_small_send(struct axiomnet_hw_ring *ring,
        axiom_small_msg_t *msg)
{
    int ret = 0;

    if (axiom_hw_send_small(ring->drvdata->dev_api, msg->header.tx.dst,
                msg->header.tx.port_type.raw, &msg->payload)) {
        ret = -1;
    }

    /* TODO: implement batch */
    axiom_hw_small_tx_push(ring->drvdata->dev_api, 1);

    return ret;
}

static void axiom_small_rx_dequeue(struct axiomnet_drvdata *drvdata)
{
    struct axiomnet_hw_ring *ring = &drvdata->small_rx_ring;
    struct axiomnet_sw_queue *sw_queue = &ring->sw_queue;
    unsigned long flags;
    uint32_t avail;
    int port;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    DPRINTF("start");

    spin_lock_irqsave(&sw_queue->queue_lock, flags);

    /* external cycle to recheck new batch */
    while ((avail = axiom_hw_small_rx_avail(ring->drvdata->dev_api)) > 0) {
        uint32_t received = 0;

        while (avail > 0) { /* something to read */
            axiom_small_msg_t *msg;

            queue_slot = eviq_free_head(&sw_queue->evi_queue);
            if (queue_slot == EVIQ_NONE) {
                break;
            }

            msg = &sw_queue->queue_desc[queue_slot];

            axiom_hw_recv_small(ring->drvdata->dev_api, &msg->header.rx.src,
                    &msg->header.rx.port_type.raw, &msg->payload);
            port = msg->header.rx.port_type.field.port;

            eviq_insert(&sw_queue->evi_queue, port);
            DPRINTF("queue insert - avail: %d queue_slot: %d port: %d", avail,
                    queue_slot, port);

            avail--;
            received++;
        }

        if (received > 0)
            axiom_hw_small_rx_pop(ring->drvdata->dev_api, received);

        DPRINTF("received: %d", received);

        if (queue_slot == EVIQ_NONE) {
            break;
        }
    }

    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);
    DPRINTF("end");
}

inline static int axiomnet_small_rx_avail(struct axiomnet_hw_ring *ring,
        int port)
{
    int avail;

    /* fetch new packets */
    axiom_small_rx_dequeue(ring->drvdata);

    avail = eviq_queue_avail(&ring->sw_queue.evi_queue, port);
    DPRINTF("queue - avail %d port: %d", avail, port);

    return avail;
}

inline static int axiomnet_small_recv(struct axiomnet_hw_ring *ring, int port,
        char __user *buffer, size_t len)
{
    struct axiomnet_sw_queue *sw_queue = &ring->sw_queue;
    axiom_small_msg_t *small_msg;
    eviq_pnt_t queue_slot;
    unsigned long flags;

    DPRINTF("start");

    if (len != sizeof(*small_msg)) {
        return -EFBIG;
    }

    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    queue_slot = eviq_remove(&sw_queue->evi_queue, port);

    /* XXX: impossible! */
    if (queue_slot == EVIQ_NONE) {
        len = -EFAULT;
        goto err;
    }
    DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);

    small_msg = &sw_queue->queue_desc[queue_slot];

    if (copy_to_user(buffer, small_msg, sizeof(*small_msg))) {
        len = -EFAULT;
        goto err;
    }

err:
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    DPRINTF("end len:%zu", len);
    return len;
}

static void axiomnet_sw_queue_free(struct axiomnet_sw_queue *queue)
{
    if (queue->queue_desc) {
        kfree(queue->queue_desc);
        queue->queue_desc = NULL;
    }

    eviq_release(&queue->evi_queue);
}

static int axiomnet_sw_queue_alloc(struct axiomnet_sw_queue *queue)
{
    int err;

    spin_lock_init(&queue->queue_lock);

    err = eviq_init(&queue->evi_queue, AXIOMNET_SW_QUEUE_NUM,
            AXIOMNET_SW_QUEUE_FREE_LEN);
    if (err) {
        return -ENOMEM;
    }


    queue->queue_desc = kcalloc(AXIOMNET_SW_QUEUE_FREE_LEN, sizeof(*(queue->queue_desc)),
            GFP_KERNEL);
    if (queue->queue_desc == NULL) {
        err = -ENOMEM;
        goto err;
    }

    return 0;
err:
    eviq_release(&queue->evi_queue);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_hw_ring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_hw_ring *ring)
{
    axiomnet_sw_queue_free(&ring->sw_queue);
}

static int axiomnet_hw_ring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_hw_ring *ring)
{
    int err;

    ring->drvdata = drvdata;

    mutex_init(&ring->mutex);
    init_waitqueue_head(&ring->wait_queue);

    err = axiomnet_sw_queue_alloc(&ring->sw_queue);
    if (err) {
        goto err;
    }

    return 0;

err:
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_probe(struct platform_device *pdev)
{
    struct axiomnet_drvdata *drvdata;
    uint32_t version;
    unsigned int off;
    unsigned long regs_pfn, regs_phys;
    int err = 0;


    DPRINTF("start");

    /* allocate our structure and fill it out */
    drvdata = kzalloc(sizeof(*drvdata), GFP_KERNEL);
    if (drvdata == NULL)
        return -ENOMEM;

    mutex_init(&drvdata->lock);
    drvdata->dev = &pdev->dev;
    platform_set_drvdata(pdev, drvdata);

    /* map device registers */
    drvdata->regs_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    drvdata->vregs = devm_ioremap_resource(&pdev->dev, drvdata->regs_res);
    if (IS_ERR(drvdata->vregs)) {
        dev_err(&pdev->dev, "could not map Axiom Network regs.\n");
        err = PTR_ERR(drvdata->vregs);
        goto free_local;
    }

    /* allocate axiom api */
    drvdata->dev_api = axiom_hw_dev_alloc(drvdata->vregs);
    if (drvdata->dev_api == NULL) {
        dev_err(&pdev->dev, "could not alloc axiom API\n");
        err = -ENOMEM;
        goto free_local;
    }

    /* XXX: to remove */
    off = ((unsigned long)drvdata->vregs) % PAGE_SIZE;
    regs_pfn = vmalloc_to_pfn(drvdata->vregs);
    regs_phys = (regs_pfn << PAGE_SHIFT) + off;

    DPRINTF("drvdata: %p", drvdata);
    DPRINTF("--- is_vmalloc_addr(%p): %d", drvdata->vregs, is_vmalloc_addr(drvdata->vregs))
    DPRINTF("--- vregs: %p regs_pfn:%lx regs_phys:%lx res->start:%zx", drvdata->vregs, regs_pfn,
            regs_phys, (size_t)drvdata->regs_res->start);

    axiomnet_disable_irq(drvdata);

    /* setup IRQ */
    drvdata->irq = platform_get_irq(pdev, 0);
    err = request_irq(drvdata->irq, axiomnet_irqhandler, IRQF_SHARED, pdev->name, drvdata);
    if (err) {
        dev_err(&pdev->dev, "could not get irq(%d) - %d\n", drvdata->irq, err);
        err = -EIO;
        goto free_hw_dev;
    }

    /* TODO: check version */
    version = ioread32(drvdata->vregs + AXIOMREG_IO_VERSION);
    IPRINTF(verbose, "version: 0x%08x", version);

    if (verbose > 1) {
        axiom_print_status_reg(drvdata->dev_api);
        axiom_print_control_reg(drvdata->dev_api);
        axiom_print_routing_reg(drvdata->dev_api);
        axiom_print_small_queue_reg(drvdata->dev_api);
    }

    /* alloc char device */
    err = axiomnet_alloc_chrdev(drvdata, &chrdev);
    if (err) {
        dev_err(&pdev->dev, "could not alloc char dev\n");
        goto free_irq;
    }

    /* init SMALL TX ring */
    err = axiomnet_hw_ring_init(drvdata, &drvdata->small_tx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init TX ring\n");
        goto free_cdev;
    }

    /* init SMALL RX ring */
    err = axiomnet_hw_ring_init(drvdata, &drvdata->small_rx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init RX ring\n");
        goto free_tx_ring;
    }

    axiomnet_enable_irq(drvdata);

#if 0
    do {
    //    iowrite32(AXIOMREG_CONTROL_LOOPBACK, drvdata->vregs + AXIOMREG_IO_CONTROL);
        axiom_small_msg_t small_msg;
        small_msg.header.tx.dst = 134;
        small_msg.header.tx.port_type.field.port = 0;
        small_msg.header.tx.port_type.field.type = 0;
        iowrite32(*((uint32_t*)(&small_msg)), drvdata->vregs +
                AXIOMREG_IO_SMALL_TX_BASE + 8*(0));
        iowrite32(*((uint32_t*)(&small_msg) + 1), drvdata->vregs +
                AXIOMREG_IO_SMALL_TX_BASE + 8*(0) + 4);

        iowrite32(1, drvdata->vregs + AXIOMREG_IO_SMALL_TX_PUSH);
    } while(0);
#endif

    IPRINTF(1, "AXIOM NIC driver loaded");
    DPRINTF("end");

    return 0;
free_tx_ring:
    axiomnet_hw_ring_release(drvdata, &drvdata->small_tx_ring);
free_cdev:
    axiomnet_destroy_chrdev(drvdata, &chrdev);
free_irq:
    free_irq(drvdata->irq, drvdata);
free_hw_dev:
    axiom_hw_dev_free(drvdata->dev_api);
free_local:
    kfree(drvdata);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_remove(struct platform_device *pdev)
{
    struct axiomnet_drvdata *drvdata = platform_get_drvdata(pdev);
    DPRINTF("start");

    axiomnet_disable_irq(drvdata);

    axiomnet_hw_ring_release(drvdata, &drvdata->small_rx_ring);
    axiomnet_hw_ring_release(drvdata, &drvdata->small_tx_ring);

    axiomnet_destroy_chrdev(drvdata, &chrdev);
    free_irq(drvdata->irq, drvdata);
    axiom_hw_dev_free(drvdata->dev_api);
    kfree(drvdata);

    IPRINTF(1, "AXIOM NIC driver unloaded");
    DPRINTF("end");
    return 0;
}

static struct platform_driver axiomnet_driver = {
    .probe = axiomnet_probe,
    .remove = axiomnet_remove,
    .driver = {
        .name = "axiom_net",
        .of_match_table = axiomnet_of_match,
    },
};

/************************ AxiomNet Char Device  ******************************/

static ssize_t axiomnet_read(struct file *filep, char __user *buffer, size_t len,
        loff_t *offset)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_hw_ring *ring = &drvdata->small_rx_ring;
    int port = priv->bind_port;

    DPRINTF("start");

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    /* TODO: use 1 mutex per port */
    if (mutex_lock_interruptible(&ring->mutex))
        return -ERESTARTSYS;

    while (axiomnet_small_rx_avail(ring, port) == 0) { /* nothing to read */
        mutex_unlock(&ring->mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(ring->wait_queue,
                    axiomnet_small_rx_avail(ring, port) != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&ring->mutex))
            return -ERESTARTSYS;
    }

    /* copy packet from the ring */
    len = axiomnet_small_recv(ring, port, buffer, len);

    /* TODO: flush rx ring */
    /* TODO: implement flush (fops) and ioctl to enable/disable auto flush */

//err:
    mutex_unlock(&ring->mutex);

    DPRINTF("end");
    return len;
}

static ssize_t axiomnet_write(struct file *filep, const char __user *buffer,
        size_t len, loff_t *offset)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_hw_ring *ring = &drvdata->small_tx_ring;
    axiom_small_msg_t small_msg;

    DPRINTF("start");
    if (mutex_lock_interruptible(&ring->mutex))
        return -ERESTARTSYS;

    while (axiomnet_small_tx_avail(ring) == 0) { /* no space to write */
        mutex_unlock(&ring->mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(ring->wait_queue,
                    axiomnet_small_tx_avail(ring) != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&ring->mutex))
            return -ERESTARTSYS;
    }

    /* XXX: we support only small message for now */
    if (len != sizeof(small_msg)) {
        len = -EFBIG;
        goto err;
    }

    if (copy_from_user(&small_msg, buffer, len)) {
        len = -EFAULT;
        goto err;
    }

    /* copy packet into the ring */
    if (axiomnet_small_send(ring, &small_msg)) {
        len = -EFAULT;
        goto err;
    }

    /* TODO: flush tx ring */
    /* TODO: implement flush (fops) and ioctl to enable/disable auto flush */

err:
    mutex_unlock(&ring->mutex);

    DPRINTF("end");
    return len;

}

static long axiomnet_bind(struct axiomnet_priv *priv, int port) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    long ret = 0;

    /* unbind previous bind */
    axiomnet_unbind(priv);

    if (port >= AXIOM_SMALL_PORT_LENGTH) {
        return -EFBIG;
    }

    mutex_lock(&drvdata->lock);

    DPRINTF("port: 0x%x port_used: 0x%x", port, drvdata->port_used);

    /* check if port is already bound */
    if (((1 << port) & drvdata->port_used)) {
        EPRINTF("Port %d already bound", port);
        ret = -EBUSY;
        goto exit;
    }

    priv->bind_port = port;
    drvdata->port_used |= (1 << (uint8_t)port);

    DPRINTF("port: 0x%x port_used: 0x%x", port, drvdata->port_used);

exit:
    mutex_unlock(&drvdata->lock);

    return ret;
}

static void axiomnet_unbind(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;

    /* check if the process bound some port */
    if (priv->bind_port == AXIOMNET_PORT_INVALID ||
            priv->bind_port >= AXIOM_SMALL_PORT_LENGTH) {
        return;
    }

    mutex_lock(&drvdata->lock);
    drvdata->port_used &= ~(1 << (uint8_t)(priv->bind_port));
    mutex_unlock(&drvdata->lock);

    priv->bind_port = AXIOMNET_PORT_INVALID;
}

static long axiomnet_ioctl(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    void __user* argp = (void __user*)arg;
    long ret = 0;
    uint32_t buf_uint32;
    uint8_t buf_uint8;
    uint8_t buf_uint8_2;
    axiom_ioctl_routing_t buf_routing;

    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    switch (cmd) {
    case AXNET_SET_NODEID:
        get_user(buf_uint8, (uint8_t __user*)arg);
        axiom_hw_set_node_id(drvdata->dev_api, buf_uint8);
        break;
    case AXNET_GET_NODEID:
        buf_uint8 = axiom_hw_get_node_id(drvdata->dev_api);
        put_user(buf_uint8, (uint8_t __user*)arg);
        break;
    case AXNET_SET_ROUTING:
        ret = copy_from_user(&buf_routing, argp, sizeof(buf_routing));
        if (ret)
            return -EFAULT;
        ret = axiom_hw_set_routing(drvdata->dev_api, buf_routing.node_id,
                buf_routing.enabled_mask);
        break;
    case AXNET_GET_ROUTING:
        ret = copy_from_user(&buf_routing, argp, sizeof(buf_routing));
        if (ret)
            return -EFAULT;
        ret = axiom_hw_get_routing(drvdata->dev_api, buf_routing.node_id,
                &buf_routing.enabled_mask);
        ret = copy_to_user(argp, &buf_routing, sizeof(buf_routing));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_GET_IFNUMBER:
        ret = axiom_hw_get_if_number(drvdata->dev_api, &buf_uint8);
        put_user(buf_uint8, (uint8_t __user*)arg);
        break;
    case AXNET_GET_IFINFO:
        get_user(buf_uint8, (uint8_t __user*)arg);
        ret = axiom_hw_get_if_info(drvdata->dev_api, buf_uint8, &buf_uint8_2);
        put_user(buf_uint8_2, (uint8_t __user*)arg);
        break;
    case AXNET_GET_STATUS:
        buf_uint32 = axiom_hw_read_ni_status(drvdata->dev_api);
        put_user(buf_uint32, (uint32_t __user*)arg);
        break;
    case AXNET_GET_CONTROL:
        buf_uint32 = axiom_hw_read_ni_control(drvdata->dev_api);
        put_user(buf_uint32, (uint32_t __user*)arg);
        break;
    case AXNET_SET_CONTROL:
        get_user(buf_uint32, (uint32_t __user*)arg);
        axiom_hw_set_ni_control(drvdata->dev_api, buf_uint32);
        break;
    case AXNET_BIND:
        get_user(buf_uint8, (uint8_t __user*)arg);
        ret = axiomnet_bind(priv, buf_uint8);
        DPRINTF("bind port: %x", priv->bind_port);
        break;
    default:
        ret = -EINVAL;
    }

    DPRINTF("end");
    return ret;
}

static int axiomnet_mmap(struct file *filep, struct vm_area_struct *vma)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    unsigned long size = vma->vm_end - vma->vm_start;
    int err = 0;
    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    mutex_lock(&drvdata->lock);
    if (size != AXIOMREG_IO_SIZE) {
        err= -EINVAL;
        goto err;
    }

    err = remap_pfn_range(vma, vma->vm_start,
            drvdata->regs_res->start >> PAGE_SHIFT, size, vma->vm_page_prot);
    if (err) {
        goto err;
    }

    mutex_unlock(&drvdata->lock);

    DPRINTF("end");
    return 0;
err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to mmap registers [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_open(struct inode *inode, struct file *filep)
{
    int err = 0;
    unsigned int minor = iminor(inode);
    struct axiomnet_drvdata *drvdata = chrdev.drvdata_a[minor];
    struct axiomnet_priv *priv;

    DPRINTF("start minor: %u drvdata: %p", minor, drvdata);

    /* allocate per-open structure and fill it out */
    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (priv == NULL)
        return -ENOMEM;

    /* set invalid port */
    priv->bind_port = AXIOMNET_PORT_INVALID;

    mutex_lock(&drvdata->lock);
    if (drvdata->used >= AXIOMNET_MAX_OPEN) {
        err = -EBUSY;
        goto err;
    }
    drvdata->used++;
    mutex_unlock(&drvdata->lock);

    priv->drvdata = drvdata;

    filep->private_data = priv;

    DPRINTF("end");
    return 0;

err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to open char dev [error %d]\n", err);
    kfree(priv);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_release(struct inode *inode, struct file *filep)
{
    unsigned int minor = iminor(inode);
    struct axiomnet_drvdata *drvdata = chrdev.drvdata_a[minor];
    struct axiomnet_priv *priv = filep->private_data;

    DPRINTF("start");

    axiomnet_unbind(priv);

    mutex_lock(&drvdata->lock);

    drvdata->used--;

    mutex_unlock(&drvdata->lock);

    filep->private_data = NULL;
    kfree(priv);

    DPRINTF("end");
    return 0;
}

static struct file_operations axiomnet_fops =
{
    .owner = THIS_MODULE,
    .open = axiomnet_open,
    .release = axiomnet_release,
    .unlocked_ioctl = axiomnet_ioctl,
    .mmap = axiomnet_mmap,
    .read = axiomnet_read,
    .write = axiomnet_write
};

static int axiomnet_alloc_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev)
{
    int err = 0, devnum;
    struct device *dev_ret;
    DPRINTF("start");

    spin_lock(&map_lock);
    devnum = find_first_zero_bit(dev_map, AXIOMNET_DEV_MAX);
    if (devnum >= AXIOMNET_DEV_MAX) {
        spin_unlock(&map_lock);
        err = -ENOMEM;
        goto err;
    }
    set_bit(devnum, dev_map);
    spin_unlock(&map_lock);

    dev_ret  = device_create(chrdev->dclass, NULL,
            MKDEV(MAJOR(chrdev->dev), devnum), drvdata,
            "%s%d",AXIOMNET_DEV_NAME, devnum);
    if (IS_ERR(dev_ret)) {
        err = PTR_ERR(dev_ret);
        goto free_devnum;
    }
    drvdata->devnum = devnum;
    drvdata->used = 0;

    chrdev->drvdata_a[devnum] = drvdata;

    DPRINTF("end major:%d minor:%d", MAJOR(chrdev->dev), devnum);
    return 0;

free_devnum:
    spin_lock(&map_lock);
    clear_bit(devnum, dev_map);
    spin_unlock(&map_lock);

err:
    pr_err("unable to allocate char dev [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev)
{
    DPRINTF("start");

    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev), drvdata->devnum));
    spin_lock(&map_lock);
    clear_bit(drvdata->devnum, dev_map);
    spin_unlock(&map_lock);

    chrdev->drvdata_a[drvdata->devnum] = NULL;
    drvdata->devnum = -1;

    DPRINTF("end");
}

static int axiomnet_init_chrdev(struct axiomnet_chrdev *chrdev)
{
    int err = 0;
    DPRINTF("start");

    err = alloc_chrdev_region(&chrdev->dev, AXIOMNET_DEV_MINOR,
                    AXIOMNET_DEV_MAX, AXIOMNET_DEV_NAME);
    if (err) {
        goto err;
    }

    cdev_init(&chrdev->cdev, &axiomnet_fops);

    err = cdev_add(&chrdev->cdev, chrdev->dev, AXIOMNET_DEV_MAX);
    if (err < 0)
    {
        goto free_dev;
    }

    chrdev->dclass = class_create(THIS_MODULE, AXIOMNET_DEV_CLASS);
    if (IS_ERR(chrdev->dclass)) {
        err = PTR_ERR(chrdev->dclass);
        goto free_cdev;
    }

    return 0;

free_cdev:
    cdev_del(&chrdev->cdev);
free_dev:
    unregister_chrdev_region(chrdev->dev, AXIOMNET_DEV_MINOR);
err:
    pr_err("unable to init char dev [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_cleanup_chrdev(struct axiomnet_chrdev *chrdev)
{
    DPRINTF("start");

    class_unregister(chrdev->dclass);
    class_destroy(chrdev->dclass);
    cdev_del(&chrdev->cdev);
    unregister_chrdev_region(chrdev->dev, AXIOMNET_DEV_MINOR);

    DPRINTF("end");
}

/********************** AxiomNet Module [un]init *****************************/

/* Entry point for loading the module */
static int __init axiomnet_init(void)
{
    int err;
    DPRINTF("start");

    err = axiomnet_init_chrdev(&chrdev);
    if (err) {
        goto err;
    }

    err = platform_driver_register(&axiomnet_driver);
    if (err) {
        goto free_chrdev;
    }

    DPRINTF("end");
    return 0;

free_chrdev:
    axiomnet_cleanup_chrdev(&chrdev);
err:
    pr_err("unable to init axiomnet module [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

/* Entry point for unloading the module */
static void __exit axiomnet_cleanup(void)
{
    DPRINTF("start");
    platform_driver_unregister(&axiomnet_driver);
    axiomnet_cleanup_chrdev(&chrdev);
    DPRINTF("end");
}

module_init(axiomnet_init);
module_exit(axiomnet_cleanup);
