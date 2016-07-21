/*!
 * \file axiom_netdev_module.c
 *
 * \version     v0.7
 * \date        2016-05-03
 *
 * This file contains the implementation of the Axiom NIC kernel module.
 */
#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
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
MODULE_VERSION("v0.7");

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



/************************ AxiomNet Device Driver ******************************/

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

    if (irq_pending & AXIOMREG_IRQ_RAW_RX) {
        axiom_kthread_wakeup(&drvdata->kthread_raw);
    }

    if (irq_pending & AXIOMREG_IRQ_RAW_TX) {
        wake_up(&(drvdata->raw_tx_ring.port.wait_queue));
    }

    if (irq_pending & AXIOMREG_IRQ_RDMA_RX) {
        axiom_kthread_wakeup(&drvdata->kthread_rdma);
    }

    if (irq_pending & AXIOMREG_IRQ_RDMA_TX) {
        wake_up(&(drvdata->rdma_tx_ring.port.wait_queue));
    }

    iowrite32(irq_pending, drvdata->vregs + AXIOMREG_IO_ACKIRQ);
    serviced = IRQ_HANDLED;
    DPRINTF("end");

    return serviced;
}

/****************************** RAW functions *********************************/

inline static int axiomnet_raw_tx_avail(struct axiomnet_raw_tx_hwring *tx_ring)
{
    /*TODO: try to minimize the register read */
    return axiom_hw_raw_tx_avail(tx_ring->drvdata->dev_api);
}

inline static int axiomnet_raw_rx_avail(struct axiomnet_raw_rx_hwring *rx_ring,
        int port)
{
    int avail;

    avail = eviq_avail(&rx_ring->sw_queue.evi_queue, port);
    DPRINTF("queue - avail %d port: %d", avail, port);

    return avail;
}

inline static int axiomnet_raw_send(struct file *filep,
        axiom_raw_hdr_t *header, const char __user *payload)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_tx_hwring *tx_ring = &drvdata->raw_tx_ring;
    axiom_payload_t raw_payload;
    int ret;

    DPRINTF("start");

    if (mutex_lock_interruptible(&tx_ring->port.mutex))
        return -ERESTARTSYS;

    while (axiomnet_raw_tx_avail(tx_ring) == 0) { /* no space to write */
        mutex_unlock(&tx_ring->port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(tx_ring->port.wait_queue,
                    axiomnet_raw_tx_avail(tx_ring) != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&tx_ring->port.mutex))
            return -ERESTARTSYS;
    }

    if (unlikely(header->tx.payload_size > sizeof(raw_payload))) {
        ret = -EFBIG;
        goto err;
    }

    ret = copy_from_user(&(raw_payload), payload, header->tx.payload_size);
    if (unlikely(ret)) {
        ret = -EFAULT;
        goto err;
    }

    /* copy packet into the ring */
    ret = axiom_hw_raw_tx(tx_ring->drvdata->dev_api, header, &(raw_payload));
    if (unlikely(ret < 0)) {
        ret = -EFAULT;
    }

err:
    mutex_unlock(&tx_ring->port.mutex);

    DPRINTF("end");

    return ret;
}

inline static void axiom_raw_rx_dequeue(struct axiomnet_raw_rx_hwring *rx_ring)
{
    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    unsigned long flags;
    uint32_t received = 0;
    int port;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    DPRINTF("start");


    /* something to read */
    while (axiom_hw_raw_rx_avail(rx_ring->drvdata->dev_api) != 0) {
        int process_wakeup = 0;
        axiom_raw_msg_t *raw_msg;

        spin_lock_irqsave(&sw_queue->queue_lock, flags);
        queue_slot = eviq_free_pop(&sw_queue->evi_queue);
        spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

        if (unlikely(queue_slot == EVIQ_NONE)) {
            break;
        }

        raw_msg = &sw_queue->queue_desc[queue_slot];

        axiom_hw_raw_rx(rx_ring->drvdata->dev_api, &(raw_msg->header),
                &(raw_msg->payload));
        port = raw_msg->header.rx.port_type.field.port;

        /* check valid port */
        if (unlikely(port < 0 || port >= AXIOM_RAW_PORT_MAX)) {
            EPRINTF("message discarded - wrong port %d", port);
            continue;
        }

        /* TODO: maybe this lock can be avoided with double check */
        mutex_lock(&rx_ring->ports[port].mutex);

        if (axiomnet_raw_rx_avail(rx_ring, port) == 0) {
            process_wakeup = 1;
        }

        spin_lock_irqsave(&sw_queue->queue_lock, flags);
        eviq_enqueue(&sw_queue->evi_queue, port, queue_slot);
        spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

        mutex_unlock(&rx_ring->ports[port].mutex);

        /* wake up process only when the queue was empty */
        if (process_wakeup) {
            wake_up(&rx_ring->ports[port].wait_queue);
        }

        DPRINTF("queue insert - received: %d queue_slot: %d port: %d", received,
                queue_slot, port);

        received++;
    }

    DPRINTF("received: %d", received);

    DPRINTF("end");
}

inline static ssize_t axiomnet_raw_recv(struct file *filep,
        axiom_raw_hdr_t *header, char __user *payload)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    int port = priv->bind_port, wakeup_kthread = 0, ret;
    ssize_t len;

    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    axiom_raw_msg_t *raw_msg;
    eviq_pnt_t queue_slot;
    unsigned long flags;

    DPRINTF("start");

    /* check bind */
    if (unlikely(port == AXIOMNET_PORT_INVALID)) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    /* TODO: use 1 mutex per port */
    if (mutex_lock_interruptible(&rx_ring->ports[port].mutex))
        return -ERESTARTSYS;

    while (axiomnet_raw_rx_avail(rx_ring, port) == 0) { /* nothing to read */
        mutex_unlock(&rx_ring->ports[port].mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(rx_ring->ports[port].wait_queue,
                    axiomnet_raw_rx_avail(rx_ring, port) != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&rx_ring->ports[port].mutex))
            return -ERESTARTSYS;
    }

    /* copy packet from the ring */
    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    queue_slot = eviq_dequeue(&sw_queue->evi_queue, port);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->ports[port].mutex);

    /* XXX: impossible! */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        len = -EFAULT;
        goto err;
    }
    DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);

    raw_msg = &sw_queue->queue_desc[queue_slot];

    if (unlikely(header->rx.payload_size < raw_msg->header.rx.payload_size)) {
        EPRINTF("payload received too big - payload: available %d - received %d",
                header->rx.payload_size, raw_msg->header.rx.payload_size);
        len = -EFBIG;
        goto free_enqueue;
    }

    memcpy(header, &(raw_msg->header), sizeof(*header));

    ret = copy_to_user(payload, &(raw_msg->payload),
            raw_msg->header.rx.payload_size);
    if (unlikely(ret)) {
        len = -EFAULT;
        goto free_enqueue;
    }

    len = sizeof(*header) + raw_msg->header.rx.payload_size;

free_enqueue:
    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    /* send a notification to kthread if the free queue was empty */
    if (eviq_free_avail(&sw_queue->evi_queue) == 0) {
        wakeup_kthread = 1;
    }
    eviq_free_push(&sw_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    if (wakeup_kthread) {
        axiom_kthread_wakeup(&drvdata->kthread_raw);
    }

err:


    DPRINTF("end len:%zu", len);
    return len;
}

static long axiomnet_raw_flush(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    int port = priv->bind_port;
    unsigned long flags;
    long ret = 0;

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    if (mutex_lock_interruptible(&rx_ring->ports[port].mutex))
        return -ERESTARTSYS;

    /* take the lock to avoid enqueue during the flush */
    spin_lock_irqsave(&sw_queue->queue_lock, flags);

    while (axiomnet_raw_rx_avail(rx_ring, port) != 0) {
        eviq_pnt_t queue_slot;

        queue_slot = eviq_dequeue(&sw_queue->evi_queue, port);

        /* XXX: impossible! */
        if (queue_slot == EVIQ_NONE) {
            ret = -EFAULT;
            goto err;
        }

        eviq_free_push(&sw_queue->evi_queue, queue_slot);

        DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);
    }

err:
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->ports[port].mutex);
    return ret;
}

/***************************** RDMA functions *********************************/
inline static int axiomnet_rdma_tx_avail(struct axiomnet_rdma_tx_hwring *tx_ring)
{
    /*TODO: try to minimize the register read */
    return axiom_hw_rdma_tx_avail(tx_ring->drvdata->dev_api) &&
        eviq_free_avail(&tx_ring->sw_queue.evi_queue);
}

inline static int axiomnet_rdma_tx(struct file *filep,
        axiom_rdma_hdr_t *header)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_queue *sw_queue = &tx_ring->sw_queue;
    axiom_rdma_status_t *rdma_status;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    unsigned long flags;
    bool wakeup_thread = false;
    int ret;

    DPRINTF("start");

    if (mutex_lock_interruptible(&tx_ring->port.mutex))
        return -ERESTARTSYS;

    /* check slot available in the HW ring */
    while (axiomnet_rdma_tx_avail(tx_ring) == 0) { /* no space to write */
        mutex_unlock(&tx_ring->port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(tx_ring->port.wait_queue,
                    axiomnet_rdma_tx_avail(tx_ring) != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&tx_ring->port.mutex))
            return -ERESTARTSYS;
    }

    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    queue_slot = eviq_free_pop(&sw_queue->evi_queue);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    /* impossible */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        ret = -EFAULT;
        goto err;
    }

    rdma_status = &(sw_queue->queue_desc[queue_slot]);
    rdma_status->ack_received = 0;
    rdma_status->remote_id = header->tx.dst;
    header->tx.msg_id = rdma_status->msg_id;

    /* copy packet into the ring */
    ret = axiom_hw_rdma_tx(drvdata->dev_api, header);
    if (unlikely(ret != header->tx.msg_id)) {
        ret = -EFAULT;
        goto err;
    }

    /* XXX: maybe we can unlock the mutex */

    /* TODO: handle if the user interrupt this function */

    /* wait the reply */
    while (rdma_status->ack_received == 0) {
        mutex_unlock(&tx_ring->port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait the ack */
        if (wait_event_interruptible(rdma_status->wait_queue,
                    rdma_status->ack_received != 0))
            return -ERESTARTSYS;

        if (mutex_lock_interruptible(&tx_ring->port.mutex))
            return -ERESTARTSYS;
    }

    rdma_status->ack_received = 0;
    rdma_status->remote_id = AXIOM_NULL_NODE;

    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    /* send a notification to other thread if the free queue was empty */
    if (eviq_free_avail(&sw_queue->evi_queue) == 0) {
        wakeup_thread = 1;
    }
    eviq_free_push(&sw_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

err:
    mutex_unlock(&tx_ring->port.mutex);

    if (wakeup_thread) {
        wake_up(&(tx_ring->port.wait_queue));
    }

    DPRINTF("end");

    return ret;
}

inline static void axiom_rdma_rx_dequeue(struct axiomnet_rdma_rx_hwring *rx_ring)
{
    axiom_rdma_hdr_t rdma_hdr;
    axiom_msg_id_t msg_id;

    /* something to read */
    while (axiom_hw_rdma_rx_avail(rx_ring->drvdata->dev_api) != 0) {
        axiom_rdma_status_t *rdma_status;

        msg_id = axiom_hw_rdma_rx(rx_ring->drvdata->dev_api, &rdma_hdr);

        if (unlikely(rdma_hdr.rx.port_type.field.s != 1)) {
            /* TODO: for now is impossible, the HW never propagate this
             * descriptor to the SW */
            EPRINTF("wrong RDMA descriptor received - discarded");
            continue;
        }

        rdma_status = &(rx_ring->tx_sw_queue->queue_desc[msg_id]);

        if ((rdma_status->ack_received == 1) || (rdma_status->remote_id !=
                rdma_hdr.rx.src)) {
            EPRINTF("unexpected RDMA descriptor received - discarded");
            continue;
        }

        rdma_status->ack_received = 1;

        /* wake up waitinig process */
        wake_up(&(rdma_status->wait_queue));
    }

}

/**************************** Worker functions ********************************/
static bool axiomnet_raw_rx_work_todo(void *data)
{
    struct axiomnet_raw_rx_hwring *rx_ring = data;
    return (axiom_hw_raw_rx_avail(rx_ring->drvdata->dev_api) != 0)
        && (eviq_free_avail(&rx_ring->sw_queue.evi_queue) != 0);
}

static void axiomnet_raw_rx_worker(void *data)
{
    struct axiomnet_raw_rx_hwring *rx_ring = data;

    /* fetch raw rx queue elements */
    axiom_raw_rx_dequeue(rx_ring);
}

static bool axiomnet_rdma_rx_work_todo(void *data)
{
    struct axiomnet_rdma_rx_hwring *rx_ring = data;
    return (axiom_hw_rdma_rx_avail(rx_ring->drvdata->dev_api) != 0);
}

static void axiomnet_rdma_rx_worker(void *data)
{
    struct axiomnet_rdma_rx_hwring *rx_ring = data;

    /* fetch rdma rx queue elements */
    axiom_rdma_rx_dequeue(rx_ring);
}

/***************************** Init functions *********************************/

static void axiomnet_raw_rx_hwring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_raw_rx_hwring *rx_ring)
{
    if (rx_ring->sw_queue.queue_desc) {
        kfree(rx_ring->sw_queue.queue_desc);
        rx_ring->sw_queue.queue_desc = NULL;
    }

    eviq_release(&rx_ring->sw_queue.evi_queue);
}

static int axiomnet_raw_rx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_raw_rx_hwring *rx_ring)
{
    int err, port;

    rx_ring->drvdata = drvdata;

    for (port = 0; port < AXIOM_RAW_PORT_MAX; port++) {
        mutex_init(&rx_ring->ports[port].mutex);
        init_waitqueue_head(&rx_ring->ports[port].wait_queue);
    }

    spin_lock_init(&rx_ring->sw_queue.queue_lock);

    err = eviq_init(&rx_ring->sw_queue.evi_queue, AXIOMNET_RAW_QUEUE_NUM,
            AXIOMNET_RAW_QUEUE_FREE_LEN);
    if (err) {
        err = -ENOMEM;
        goto err;
    }

    rx_ring->sw_queue.queue_desc = kcalloc(AXIOMNET_RAW_QUEUE_FREE_LEN,
            sizeof(*(rx_ring->sw_queue.queue_desc)), GFP_KERNEL);
    if (rx_ring->sw_queue.queue_desc == NULL) {
        err = -ENOMEM;
        goto release_eviq;
    }

    return 0;

release_eviq:
    eviq_release(&rx_ring->sw_queue.evi_queue);
err:
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_raw_tx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_raw_tx_hwring *tx_ring)
{
    tx_ring->drvdata = drvdata;

    mutex_init(&tx_ring->port.mutex);
    init_waitqueue_head(&tx_ring->port.wait_queue);
    return 0;
}

static void axiomnet_rdma_tx_hwring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_rdma_tx_hwring *tx_ring)
{
    if (tx_ring->sw_queue.queue_desc) {
        kfree(tx_ring->sw_queue.queue_desc);
        tx_ring->sw_queue.queue_desc = NULL;
    }

    eviq_release(&tx_ring->sw_queue.evi_queue);
}

static int axiomnet_rdma_rx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_rdma_rx_hwring *rx_ring)
{
    rx_ring->drvdata = drvdata;
    rx_ring->tx_sw_queue = &(drvdata->rdma_tx_ring.sw_queue);

    return 0;
}

static int axiomnet_rdma_tx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_rdma_tx_hwring *tx_ring)
{
    int err, i;

    tx_ring->drvdata = drvdata;

    mutex_init(&tx_ring->port.mutex);
    init_waitqueue_head(&tx_ring->port.wait_queue);

    spin_lock_init(&tx_ring->sw_queue.queue_lock);

    err = eviq_init(&tx_ring->sw_queue.evi_queue, AXIOMNET_RDMA_QUEUE_NUM,
            AXIOMNET_RDMA_QUEUE_FREE_LEN);
    if (err) {
        err = -ENOMEM;
        goto err;
    }

    tx_ring->sw_queue.queue_desc = kcalloc(AXIOMNET_RDMA_QUEUE_FREE_LEN,
            sizeof(*(tx_ring->sw_queue.queue_desc)), GFP_KERNEL);
    if (tx_ring->sw_queue.queue_desc == NULL) {
        err = -ENOMEM;
        goto release_eviq;
    }

    for (i = 0; i < AXIOMNET_RDMA_QUEUE_FREE_LEN; i++) {
        tx_ring->sw_queue.queue_desc[i].msg_id = i;
        tx_ring->sw_queue.queue_desc[i].remote_id = AXIOM_NULL_NODE;
        init_waitqueue_head(&(tx_ring->sw_queue.queue_desc[i].wait_queue));
    }

    return 0;

release_eviq:
    eviq_release(&tx_ring->sw_queue.evi_queue);
err:
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_rdma_release(struct axiomnet_drvdata *drvdata)
{
    dma_free_coherent(drvdata->dev, drvdata->dma_size, drvdata->dma_vaddr,
            drvdata->dma_paddr);
    drvdata->dma_vaddr = NULL;
    drvdata->dma_paddr = 0;
    drvdata->dma_size = 0;
}

static int axiomnet_rdma_init(struct axiomnet_drvdata *drvdata)
{
    int ret;

    drvdata->dma_size = (1 << 24);

    drvdata->dma_vaddr = dma_zalloc_coherent(drvdata->dev, drvdata->dma_size,
            &drvdata->dma_paddr, GFP_KERNEL);
    if (drvdata->dma_vaddr == NULL) {
        ret = -ENOMEM;
        goto err;
    }

    IPRINTF(1, "DMA mapped - vaddr 0x%p paddr 0x%llx size 0x%llx",
            drvdata->dma_vaddr, drvdata->dma_paddr, drvdata->dma_size);

    axiom_hw_set_rdma_zone(drvdata->dev_api, drvdata->dma_paddr,
            drvdata->dma_paddr + drvdata->dma_size - 1);

    return 0;
err:
    DPRINTF("error: %d", ret);
    return ret;
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
    DPRINTF("--- is_vmalloc_addr(%p): %d", drvdata->vregs,
            is_vmalloc_addr(drvdata->vregs))
    DPRINTF("--- vregs: %p regs_pfn:%lx regs_phys:%lx res->start:%zx",
            drvdata->vregs, regs_pfn, regs_phys,
            (size_t)drvdata->regs_res->start);

    axiomnet_disable_irq(drvdata);

    /* setup IRQ */
    drvdata->irq = platform_get_irq(pdev, 0);
    err = request_irq(drvdata->irq, axiomnet_irqhandler, IRQF_SHARED,
            pdev->name, drvdata);
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
        axiom_print_queue_reg(drvdata->dev_api);
    }

    /* alloc char device */
    err = axiomnet_alloc_chrdev(drvdata, &chrdev);
    if (err) {
        dev_err(&pdev->dev, "could not alloc char dev\n");
        goto free_irq;
    }

    /* init RAW TX ring */
    err = axiomnet_raw_tx_hwring_init(drvdata, &drvdata->raw_tx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init raw TX ring\n");
        goto free_cdev;
    }

    /* init RDMA TX ring */
    err = axiomnet_rdma_tx_hwring_init(drvdata, &drvdata->rdma_tx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init RDMA TX ring\n");
        goto free_cdev;
    }

    /* init RAW RX ring */
    err = axiomnet_raw_rx_hwring_init(drvdata, &drvdata->raw_rx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init raw RX ring\n");
        goto free_tx_ring;
    }

    /* init RDMA RX ring */
    err = axiomnet_rdma_rx_hwring_init(drvdata, &drvdata->rdma_rx_ring);
    if (err) {
        dev_err(&pdev->dev, "could not init RDMA RX ring\n");
        goto free_rx_ring;
    }

    /* init RAW kthread */
    err = axiom_kthread_init(&drvdata->kthread_raw, axiomnet_raw_rx_worker,
            axiomnet_raw_rx_work_todo, &drvdata->raw_rx_ring, "RAW kthread");
    if (err) {
        dev_err(&pdev->dev, "could not init kthread\n");
        goto free_rx_ring;
    }

    /* init RDMA kthread */
    err = axiom_kthread_init(&drvdata->kthread_rdma, axiomnet_rdma_rx_worker,
            axiomnet_rdma_rx_work_todo, &drvdata->rdma_rx_ring, "RDMA kthread");
    if (err) {
        dev_err(&pdev->dev, "could not init kthread\n");
        goto free_raw_kthread;
    }

    /* init RDMA */
    err = axiomnet_rdma_init(drvdata);
    if (err) {
        dev_err(&pdev->dev, "could not init RDMA zone\n");
        goto free_rdma_kthread;
    }

    axiomnet_enable_irq(drvdata);

    IPRINTF(1, "AXIOM NIC driver loaded");
    DPRINTF("end");

    return 0;
free_rdma_kthread:
    axiom_kthread_uninit(&drvdata->kthread_rdma);
free_raw_kthread:
    axiom_kthread_uninit(&drvdata->kthread_raw);
free_rx_ring:
    axiomnet_raw_rx_hwring_release(drvdata, &drvdata->raw_rx_ring);
free_tx_ring:
    axiomnet_rdma_tx_hwring_release(drvdata, &drvdata->rdma_tx_ring);
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

    axiomnet_rdma_release(drvdata);

    axiom_kthread_uninit(&drvdata->kthread_raw);

    axiomnet_raw_rx_hwring_release(drvdata, &drvdata->raw_rx_ring);
    axiomnet_rdma_tx_hwring_release(drvdata, &drvdata->rdma_tx_ring);

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

/****************************** Ports Handling  *******************************/

static void axiomnet_unbind(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;

    /* check if the process bound some port */
    if (priv->bind_port == AXIOMNET_PORT_INVALID ||
            priv->bind_port >= AXIOM_RAW_PORT_MAX) {
        return;
    }

    mutex_lock(&drvdata->lock);
    drvdata->port_used &= ~(1 << (uint8_t)(priv->bind_port));
    mutex_unlock(&drvdata->lock);

    priv->bind_port = AXIOMNET_PORT_INVALID;
}

static long axiomnet_bind(struct axiomnet_priv *priv, uint8_t *port) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    long ret = 0;

    /* unbind previous bind */
    axiomnet_unbind(priv);

    mutex_lock(&drvdata->lock);

    if (*port == AXIOM_PORT_ANY) {
        int i;
        /* assign first port available */
        for (i = 0; i < AXIOM_RAW_PORT_MAX; i++) {
            if (!((1 << i) & drvdata->port_used)) {
                *port = i;
                break;
            }
        }

        if (*port == AXIOM_PORT_ANY) {
            ret = -EBUSY;
            goto exit;
        }
    } else if (*port >= AXIOM_RAW_PORT_MAX) {
        ret = -EFBIG;
        goto exit;
    }


    DPRINTF("port: 0x%x port_used: 0x%x", *port, drvdata->port_used);

    /* check if port is already bound */
    if (((1 << *port) & drvdata->port_used)) {
        EPRINTF("Port %d already bound", *port);
        ret = -EBUSY;
        goto exit;
    }

    priv->bind_port = *port;
    drvdata->port_used |= (1 << (uint8_t)*port);

    DPRINTF("port: 0x%x port_used: 0x%x", *port, drvdata->port_used);

exit:
    mutex_unlock(&drvdata->lock);

    return ret;
}

inline static int axiomnet_check_port(struct axiomnet_priv *priv)
{
    int port = priv->bind_port;

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    return port;
}

/************************ AxiomNet Char Device  ******************************/

static long axiomnet_ioctl(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_rx_hwring *rx_ring;
    struct axiomnet_raw_tx_hwring *tx_ring;
    void __user* argp = (void __user*)arg;
    long ret = 0;
    uint32_t buf_uint32;
    uint64_t buf_uint64;
    int buf_int, port;
    uint8_t buf_uint8;
    uint8_t buf_uint8_2;
    axiom_ioctl_routing_t buf_routing;
    axiom_ioctl_raw_t buf_raw;
    axiom_ioctl_bind_t buf_bind;
    axiom_rdma_hdr_t buf_rdma;

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
        ret = copy_from_user(&buf_bind, argp, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        ret = axiomnet_bind(priv, &(buf_bind.port));
        DPRINTF("bind port: %x flush: %x", priv->bind_port, buf_bind.flush);
        if (ret)
            return ret;
        /* flush all previous received packets */
        if (buf_bind.flush) {
            ret = axiomnet_raw_flush(priv);
        }
        ret = copy_to_user(argp, &buf_bind, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_RAW:
        ret = copy_from_user(&buf_raw, argp, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        ret = axiomnet_raw_send(filep, &(buf_raw.header),
                buf_raw.payload);
        break;
    case AXNET_RECV_RAW:
        ret = copy_from_user(&buf_raw, argp, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        ret = axiomnet_raw_recv(filep, &(buf_raw.header),
                buf_raw.payload);
        if (ret < 0)
            return ret;
        ret = copy_to_user(argp, &buf_raw, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_RAW_AVAIL:
        tx_ring = &drvdata->raw_tx_ring;
        buf_int = axiomnet_raw_tx_avail(tx_ring);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_RECV_RAW_AVAIL:
        rx_ring = &drvdata->raw_rx_ring;
        port = axiomnet_check_port(priv);
        if (port < 0)
            return port;
        buf_int = axiomnet_raw_rx_avail(rx_ring, port);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_FLUSH_RAW:
        ret = axiomnet_raw_flush(priv);
        break;
    case AXNET_RDMA_SIZE:
        buf_uint64 = drvdata->dma_size;
        put_user(buf_uint64, (uint64_t __user*)arg);
        break;
    case AXNET_RDMA_WRITE:
    case AXNET_RDMA_READ:
        ret = copy_from_user(&buf_rdma, argp, sizeof(buf_rdma));
        if (ret)
            return -EFAULT;
        ret = axiomnet_rdma_tx(filep, &(buf_rdma));
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

    if (!drvdata || !drvdata->dma_paddr)
        return -EINVAL;

    mutex_lock(&drvdata->lock);

    if (size != drvdata->dma_size) {
        err= -EINVAL;
        goto err;
    }

    err = remap_pfn_range(vma, vma->vm_start,
            drvdata->dma_paddr >> PAGE_SHIFT, size, vma->vm_page_prot);
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
