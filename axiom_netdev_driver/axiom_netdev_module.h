#ifndef AXIOM_NETDEV_MODULE_H
#define AXIOM_NETDEV_MODULE_H

#include "dprintf.h"
#include "axiom_nic_regs.h"
#include "axiom_kernel_api.h"

#define AXIOMNET_DEV_MINOR      0
#define AXIOMNET_DEV_MAX        8
#define AXIOMNET_DEV_NAME       "axiom"
#define AXIOMNET_DEV_CLASS      "axiomchar"

#define AXIOMNET_MAX_OPEN       1

struct axiomnet_ring {
    struct axiomnet_drvdata *drvdata;

    unsigned int desc_count;
    unsigned int next_to_use;
    unsigned int next_to_clean;

    //spinlock_t lock;
    struct mutex mutex;
    wait_queue_head_t wait_queue;
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
    struct axiomnet_ring small_tx_ring;

    /* SMALL RX Rings */
    struct axiomnet_ring small_rx_ring;


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

#endif /* AXIOM_NETDEV_MODULE_H */
