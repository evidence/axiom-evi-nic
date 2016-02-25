#ifndef AXIOM_NETDEV_H
#define AXIOM_NETDEV_H

#include "axiom_netdev_hw.h"

#define AXIOMNET_DEV_MINOR      0
#define AXIOMNET_DEV_MAX        8
#define AXIOMNET_DEV_NAME       "axiom"
#define AXIOMNET_DEV_CLASS      "axiomchar"

#define AXIOMNET_MAX_OPEN       1

struct axiomnet_buffer {
    void *addr;
    dma_addr_t dma;
};

struct axiomnet_ring {
    struct axiom_netdev_bufdesc *bufdesc;
    dma_addr_t dma;
    unsigned int size;
    unsigned int count;
    unsigned int next_to_use;
    unsigned int next_to_clean;
    struct axiomnet_buffer *buffer;

};


struct axiomnet_drvdata {
    struct device *dev;

    /* IO registers */
    void __iomem *vregs;                /* kernel virtual address (vmalloc)*/
    struct resource *regs_res;

    /* IRQ */
    int irq;

    /* TX Rings TODO: */
    struct axiomnet_ring tx_ring;

    /* RX Rings */
    struct axiomnet_ring rx_ring;


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
