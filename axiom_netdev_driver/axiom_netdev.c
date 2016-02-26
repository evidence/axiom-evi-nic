#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/types.h>

#include "axiom_netdev.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence S.R.L. - Stefano Garzarella");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("2016-01-22");

int debug = 0;
module_param(debug, int, 0);
MODULE_PARM_DESC(debug, "Debug level (0=none,...,16=all)");

/* bitmat to handle chardev minor (devnum) */
static DEFINE_SPINLOCK(map_lock);
static DECLARE_BITMAP(dev_map, AXIOMNET_DEV_MAX);

struct axiomnet_chrdev chrdev;




static int axiomnet_alloc_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);
static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);

/*********************** AxiomNet Device Driver ******************************/

/* Match table for of_platform binding */
static const struct of_device_id axiomnet_of_match[] = {
    { .compatible = AXIOMNET_COMPATIBLE, },
    {},
};

static inline void axiomnet_enable_irq(struct axiomnet_drvdata *drvdata)
{
    iowrite32(AXIOMNET_IRQ_ALL, drvdata->vregs + AXIOMNET_IO_SETIRQ);
}

static inline void axiomnet_disable_irq(struct axiomnet_drvdata *drvdata)
{
    iowrite32(~AXIOMNET_IRQ_ALL, drvdata->vregs + AXIOMNET_IO_SETIRQ);
}

static int axiomnet_setup_ring(struct axiomnet_drvdata *drvdata,
                                struct axiomnet_ring *ring,
                                unsigned int size, unsigned int count)
{
    ring->total_size = ALIGN(size * count, PAGE_SIZE);

    ring->desc_addr = dma_alloc_coherent(drvdata->dev, ring->total_size,
            &ring->desc_dma, GFP_KERNEL);

    if (!ring->desc_addr) {
        ring->total_size = 0;
        return -ENOMEM;
    }

    ring->desc_size = size;
    ring->desc_count = count;
    ring->next_to_use = 0;
    ring->next_to_clean = 0;

    return 0;
}

static void axiomnet_destroy_ring(struct axiomnet_drvdata *drvdata,
                              struct axiomnet_ring *ring)
{
    if (!ring->desc_addr)
        return;

    dma_free_coherent(drvdata->dev, ring->total_size, ring->desc_addr,
            ring->desc_dma);

    ring->desc_addr = NULL;
    ring->total_size = 0;


}

static int axiomnet_alloc_rings(struct axiomnet_drvdata *drvdata)

{
    int err = 0;

    /* setup RAW TX queue */
    err = axiomnet_setup_ring(drvdata, &drvdata->raw_tx_ring,
            sizeof(struct axiomRawMsg), AXIOMNET_DEF_RING_LEN);

    return err;
}

static int axiomnet_free_rings(struct axiomnet_drvdata *drvdata)

{
    axiomnet_destroy_ring(drvdata, &drvdata->raw_tx_ring);

    return 0;
}

static irqreturn_t axiomnet_irqhandler(int irq, void *dev_id)
{
    struct axiomnet_drvdata *drvdata = dev_id;
    irqreturn_t serviced = IRQ_NONE;
    uint32_t irq_pending;

    dprintk("start");
    irq_pending = ioread32(drvdata->vregs + AXIOMNET_IO_PNDIRQ);
    iowrite32(irq_pending, drvdata->vregs + AXIOMNET_IO_ACKIRQ);
    serviced = IRQ_HANDLED;
    dprintk("end");

    return serviced;
}

static int axiomnet_probe(struct platform_device *pdev)
{
    struct axiomnet_drvdata *drvdata;
    uint32_t features;
    unsigned int off;
    unsigned long regs_pfn, regs_phys;
    int err = 0;


    dprintk("start");

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

    /* to remove */
    off = ((unsigned long)drvdata->vregs) % PAGE_SIZE;
    regs_pfn = vmalloc_to_pfn(drvdata->vregs);
    regs_phys = (regs_pfn << PAGE_SHIFT) + off;

    dprintk("drvdata: %p", drvdata);
    dprintk("--- is_vmalloc_addr(%p): %d", drvdata->vregs, is_vmalloc_addr(drvdata->vregs))
    dprintk("--- vregs: %p regs_pfn:%lx regs_phys:%lx res->start:%zx", drvdata->vregs, regs_pfn,
            regs_phys, drvdata->regs_res->start);

    axiomnet_disable_irq(drvdata);

    /* setup IRQ */
    drvdata->irq = platform_get_irq(pdev, 0);
    err = request_irq(drvdata->irq, axiomnet_irqhandler, IRQF_SHARED, pdev->name, drvdata);

    /* TODO: check features */
    features = ioread32(drvdata->vregs + AXIOMNET_IO_FEATURES);
    sprintk("features: %zu", features);

    /* alloc char device */
    err = axiomnet_alloc_chrdev(drvdata, &chrdev);
    if (err) {
        goto free_irq;
    }

    /* setup rings */
    err = axiomnet_alloc_rings(drvdata);
    if (err) {
        goto free_cdev;
    }



    axiomnet_enable_irq(drvdata);

    dprintk("end");

    return 0;
free_cdev:
    axiomnet_destroy_chrdev(drvdata, &chrdev);
free_irq:
    free_irq(drvdata->irq, drvdata);
free_local:
    kfree(drvdata);
    dprintk("error: %d", err);
    return err;
}

static int axiomnet_remove(struct platform_device *pdev)
{
    struct axiomnet_drvdata *drvdata = platform_get_drvdata(pdev);
    dprintk("start");

    axiomnet_free_rings(drvdata);
    axiomnet_destroy_chrdev(drvdata, &chrdev);
    free_irq(drvdata->irq, drvdata);
    kfree(drvdata);

    dprintk("end");
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

static int axiomnet_open(struct inode *inode, struct file *filep)
{
    int err = 0;
    unsigned int minor = iminor(inode);
    struct axiomnet_drvdata *drvdata = chrdev.drvdata_a[minor];
    dprintk("start minor: %u drvdata: %p", minor, drvdata);

    mutex_lock(&drvdata->lock);

    if (drvdata->used >= AXIOMNET_MAX_OPEN) {
        err = -EBUSY;
        goto err;
    }
    drvdata->used++;

    filep->private_data = drvdata;

    mutex_unlock(&drvdata->lock);
    dprintk("end");
    return 0;

err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to open char dev [error %d]\n", err);
    dprintk("error: %d", err);
    return err;
}
static int axiomnet_release(struct inode *inode, struct file *filep)
{
    unsigned int minor = iminor(inode);
    struct axiomnet_drvdata *drvdata = chrdev.drvdata_a[minor];
    dprintk("start");

    mutex_lock(&drvdata->lock);

    drvdata->used--;

    mutex_unlock(&drvdata->lock);

    dprintk("end");
    return 0;
}

static ssize_t axiomnet_read(struct file *filep, char __user *buffer, size_t len,
        loff_t *offset)
{
    dprintk("start");
    dprintk("end");
    return 0;
}

static ssize_t axiomnet_write(struct file *filep, const char __user *buffer,
        size_t len, loff_t *offset)
{
    dprintk("start");
    dprintk("end");
    return len;
}


static long axiomnet_ioctl(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    dprintk("start");
    dprintk("end");
    return 0;
}

static int axiomnet_mmap(struct file *filep, struct vm_area_struct *vma)
{
    struct axiomnet_drvdata *drvdata = filep->private_data;
    unsigned long size = vma->vm_end - vma->vm_start;
    int err = 0;
    dprintk("start");

    mutex_lock(&drvdata->lock);
    if (size != AXIOMNET_IO_SIZE) {
        err= -EINVAL;
        goto err;
    }

    err = remap_pfn_range(vma, vma->vm_start,
            drvdata->regs_res->start >> PAGE_SHIFT, size, vma->vm_page_prot);
    if (err) {
        goto err;
    }

    mutex_unlock(&drvdata->lock);

    dprintk("end");
    return 0;
err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to mmap registers [error %d]\n", err);
    dprintk("error: %d", err);
    return err;
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
    dprintk("start");

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

    dprintk("end major:%d minor:%d", MAJOR(chrdev->dev), devnum);
    return 0;

free_devnum:
    spin_lock(&map_lock);
    clear_bit(devnum, dev_map);
    spin_unlock(&map_lock);

err:
    pr_err("unable to allocate char dev [error %d]\n", err);
    dprintk("error: %d", err);
    return err;
}

static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev)
{
    dprintk("start");

    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev), drvdata->devnum));
    spin_lock(&map_lock);
    clear_bit(drvdata->devnum, dev_map);
    spin_unlock(&map_lock);

    chrdev->drvdata_a[drvdata->devnum] = NULL;
    drvdata->devnum = -1;

    dprintk("end");
}

static int axiomnet_init_chrdev(struct axiomnet_chrdev *chrdev)
{
    int err = 0;
    dprintk("start");

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
    dprintk("error: %d", err);
    return err;
}

static void axiomnet_cleanup_chrdev(struct axiomnet_chrdev *chrdev)
{
    dprintk("start");

    class_unregister(chrdev->dclass);
    class_destroy(chrdev->dclass);
    cdev_del(&chrdev->cdev);
    unregister_chrdev_region(chrdev->dev, AXIOMNET_DEV_MINOR);

    dprintk("end");
}

/********************** AxiomNet Module [un]init *****************************/

/* Entry point for loading the module */
static int __init axiomnet_init(void)
{
    int err;
    dprintk("start");

    err = axiomnet_init_chrdev(&chrdev);
    if (err) {
        goto err;
    }

    err = platform_driver_register(&axiomnet_driver);
    if (err) {
        goto free_chrdev;
    }

    dprintk("end");
    return 0;

free_chrdev:
    axiomnet_cleanup_chrdev(&chrdev);
err:
    pr_err("unable to init axiomnet module [error %d]\n", err);
    dprintk("error: %d", err);
    return err;
}

/* Entry point for unloading the module */
static void __exit axiomnet_cleanup(void)
{
    dprintk("start");
    platform_driver_unregister(&axiomnet_driver);
    axiomnet_cleanup_chrdev(&chrdev);
    dprintk("end");
}

module_init(axiomnet_init);
module_exit(axiomnet_cleanup);
