#ifndef AXIOM_NETDEV_H
#define AXIOM_NETDEV_H

#include "axiom_nic_regs.h"
#include "axiom_kernel_api.h"

#define AXIOMNET_DEV_MINOR      0
#define AXIOMNET_DEV_MAX        8
#define AXIOMNET_DEV_NAME       "axiom"
#define AXIOMNET_DEV_CLASS      "axiomchar"

#define AXIOMNET_MAX_OPEN       1

#define AXIOMNET_SMALL_DESC_SIZE  sizeof(struct axiomRawMsg)

struct axiomnet_ring {
    void *desc_addr;
    dma_addr_t desc_dma;

    unsigned int total_size;
    unsigned int desc_size;
    unsigned int desc_count;
    unsigned int next_to_use;
    unsigned int next_to_clean;
};


struct axiomnet_drvdata {
    struct device *dev;

    /* Axiom Dev API */
    axiom_dev_t *dev_api;

    /* IO registers */
    void __iomem *vregs;                /* kernel virtual address (vmalloc)*/
    struct resource *regs_res;

    /* IRQ */
    int irq;

    /* SMALL TX Rings TODO */
    struct axiomnet_ring raw_tx_ring;

    /* RX Rings */
    //struct axiomnet_ring rx_ring;


    /* char dev */
    int devnum;         /* minor */
    //spinlock_t lock;
    struct mutex lock;
    int used;

};

struct axiomnet_chrdev {
    dev_t dev;
    struct class *dclass;
    struct cdev cdev;
    struct axiomnet_drvdata *drvdata_a[AXIOMNET_DEV_MAX];
};

struct axiomnet_priv {
    struct axiomnet_drvdata *drvdata;

    int bind_port;

};


#define _dprintk(_fmt, ... )\
    do {                                                        \
        struct timeval _t0;                                     \
        do_gettimeofday(&_t0);                                  \
        printk(KERN_ERR "%03d.%06d %s():%d - " _fmt "%s\n",     \
                (int)(_t0.tv_sec % 1000), (int)_t0.tv_usec,     \
                __func__, __LINE__, __VA_ARGS__);               \
    } while (0);
#define sprintk(...) _dprintk(__VA_ARGS__, "")
#define PDEBUG
#ifdef PDEBUG
#define dprintk(...) _dprintk(__VA_ARGS__, "")
#else
#define dprintk(...) do {} while(0);
#endif



#endif /* AXIOM_NETDEV_H */
