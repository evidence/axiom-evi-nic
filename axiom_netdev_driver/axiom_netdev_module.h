#ifndef AXIOM_NETDEV_MODULE_H
#define AXIOM_NETDEV_MODULE_H

#include "dprintf.h"
#include "axiom_nic_regs.h"
#include "axiom_kernel_api.h"

#define AXIOMNET_DEV_MINOR      0
#define AXIOMNET_DEV_MAX        8
#define AXIOMNET_DEV_NAME       "axiom"
#define AXIOMNET_DEV_CLASS      "axiomchar"

#define AXIOMNET_MAX_OPEN       16

#define AXIOMNET_QUEUE_LENGTH   256

struct axiomnet_ring_elem {
    axiom_small_msg_t small_msg;
    struct list_head list;
};

struct axiomnet_ring {
    struct axiomnet_drvdata *drvdata;

    /* hardware interface */
    spinlock_t queue_lock;
    struct list_head queue_empty;
    struct list_head queue_full;
    int queue_length;

    /* user-space interface */
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
