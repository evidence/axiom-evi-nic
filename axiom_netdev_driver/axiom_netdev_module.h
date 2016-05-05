#ifndef AXIOM_NETDEV_MODULE_H
#define AXIOM_NETDEV_MODULE_H
/*!
 * \file axiom_netdev_module.h
 *
 * \version     v0.4
 * \date        2016-05-03
 *
 * This file contains the structures and macros for the Axiom NIC kernel module.
 */

#include "dprintf.h"
#include "axiom_nic_regs.h"
#include "axiom_kernel_api.h"

/*! \brief AXIOM char device minor */
#define AXIOMNET_DEV_MINOR      0
/*! \brief AXIOM maximum char devices */
#define AXIOMNET_DEV_MAX        8
/*! \brief AXIOM char device base name */
#define AXIOMNET_DEV_NAME       "axiom"
/*! \brief AXIOM char device class */
#define AXIOMNET_DEV_CLASS      "axiomchar"
/*! \brief max concurrent open allowed on an AXIOM char device */
#define AXIOMNET_MAX_OPEN       16

/*! \brief number of AXIOM software queue */
#define AXIOMNET_SW_QUEUE_NUM           8
/*! \brief number of free elements in the AXIOM free queue */
#define AXIOMNET_SW_QUEUE_FREE_LEN      (256 * AXIOMNET_SW_QUEUE_NUM)

/*! \brief Invalid number of AXIOM port */
#define AXIOMNET_PORT_INVALID   -1

/*! \brief Structure to handle an AXIOM software queue */
struct axiomnet_sw_queue {
    spinlock_t queue_lock;              /*!< \brief queue lock */
    evi_queue_t evi_queue;              /*!< \brief queue manager */
    axiom_small_msg_t *queue_desc;      /*!< \brief queue elements */
};

/*! \brief Structure to handle an AXIOM hardware ring */
struct axiomnet_hw_ring {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */
    struct axiomnet_sw_queue sw_queue;  /*!< \brief AXIOM software queue */
    struct mutex mutex;                 /*!< \brief ring mutex */
    wait_queue_head_t wait_queue;       /*!< \brief ring wait queue */
};

/*! \brief AXIOM device driver data */
struct axiomnet_drvdata {
    struct device *dev;                 /*!< \brief parent device */
    axiom_dev_t *dev_api;               /*!< \brief AXIOM dev HW API*/
    /* IO registers */
    void __iomem *vregs;                /*!< \brief Memory mapped IO registers:
                                                    virtual kernel address */
    struct resource *regs_res;          /*!< \brief IO resource */
    /* IRQ */
    int irq;                            /*!< \brief IRQ descriptor */
    /*!\brief SMALL TX hardware ring */
    struct axiomnet_hw_ring small_tx_ring;
    /*!\brief SMALL RX hardware ring */
    struct axiomnet_hw_ring small_rx_ring;
    int devnum;                         /*!< \brief CharDev minor number */
    struct mutex lock;                  /*!< \brief Axiom driver mutex */
    int used;                           /*!< \brief Current number of open() */
};

/*! \brief AXIOM char device status */
struct axiomnet_chrdev {
    dev_t dev;                          /*!< \brief Parent device */
    struct class *dclass;               /*!< \brief Device Class */
    struct cdev cdev;                   /*!< \biref Linux char dev */
    /*! \brief AXIOM device driver data */
    /* TODO: maybe we have only 1 device */
    struct axiomnet_drvdata *drvdata_a[AXIOMNET_DEV_MAX];
};

/*! \brief AXIOM private data for each open */
struct axiomnet_priv {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM device driver data */
    int bind_port;                      /*!< \biref Port bound to the process */
};

#endif /* AXIOM_NETDEV_MODULE_H */
