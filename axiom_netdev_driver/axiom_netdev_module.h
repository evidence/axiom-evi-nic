#ifndef AXIOM_NETDEV_MODULE_H
#define AXIOM_NETDEV_MODULE_H
/*!
 * \file axiom_netdev_module.h
 *
 * \version     v0.7
 * \date        2016-05-03
 *
 * This file contains the structures and macros for the Axiom NIC kernel module.
 */

#include "dprintf.h"
#include "axiom_nic_regs.h"
#include "axiom_kernel_api.h"
#include "axiom_kthread.h"

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

/*! \brief number of AXIOM software RAW queue */
#define AXIOMNET_RAW_QUEUE_NUM           AXIOM_PORT_MAX
/*! \brief number of free elements in the AXIOM free RAW queue */
#define AXIOMNET_RAW_QUEUE_FREE_LEN      (256 * AXIOMNET_RAW_QUEUE_NUM)

/*! \brief number of AXIOM software RDMA queue */
#define AXIOMNET_RDMA_QUEUE_NUM          0
/*! \brief number of free elements in the AXIOM free RDMA queue */
#define AXIOMNET_RDMA_QUEUE_FREE_LEN     AXIOM_MSG_ID_MAX

/*! \brief number of AXIOM software LONG TX queue */
#define AXIOMNET_LONG_TXQUEUE_NUM        0
/*! \brief number of free elements in the AXIOM free LONG TX queue */
#define AXIOMNET_LONG_TXQUEUE_FREE_LEN   AXIOMREG_LEN_LONG_BUF

/*! \brief number of AXIOM software LONG RX queue */
#define AXIOMNET_LONG_RXQUEUE_NUM        AXIOM_PORT_MAX
/*! \brief number of free elements in the AXIOM free LONG RX queue */
#define AXIOMNET_LONG_RXQUEUE_FREE_LEN   AXIOMREG_LEN_LONG_BUF

/*! \brief Invalid number of AXIOM port */
#define AXIOMNET_PORT_INVALID           -1

struct axiomnet_drvdata;

/*! \brief AXIOM RDMA callback */
typedef void (*axiom_callback_fn_t)(struct axiomnet_drvdata *drvdata, void *data);

typedef struct axiom_callback {
    axiom_callback_fn_t func;
    void *data;
} axiom_callback_t;

/*! \brief Structure to handle msg id assignment */
typedef struct axiom_rdma_status {
    wait_queue_head_t wait_queue;       /*!< \brief wait queue */
    bool ack_received;                  /*!< \brief Is ack received? */
    bool ack_waiting;
    axiom_msg_id_t msg_id;
    eviq_pnt_t queue_slot;
    axiom_node_id_t remote_id;
    axiom_callback_t callback;
} axiom_rdma_status_t;

/*! \brief Structure to handle an AXIOM software RAW queue */
struct axiomnet_raw_queue {
    spinlock_t queue_lock;              /*!< \brief queue lock */
    evi_queue_t evi_queue;              /*!< \brief queue manager */
    axiom_raw_msg_t *queue_desc;        /*!< \brief queue elements */
};

/*! \brief Structure to handle an AXIOM software LONG queue */
struct axiomnet_long_queue {
    spinlock_t queue_lock;              /*!< \brief queue lock */
    evi_queue_t evi_queue;              /*!< \brief queue manager */
    axiom_long_msg_t *queue_desc;        /*!< \brief queue elements */
};

/*! \brief Structure to handle an AXIOM software RDMA queue */
struct axiomnet_rdma_queue {
    spinlock_t queue_lock;              /*!< \brief queue lock */
    evi_queue_t evi_queue;              /*!< \brief queue manager */
    axiom_rdma_status_t *queue_desc;    /*!< \brief queue elements */
};

/*!< \brief AXIOM struct to handle software port */
struct axiomnet_sw_port {
    struct mutex mutex;                 /*!< \brief port mutex */
    wait_queue_head_t wait_queue;       /*!< \brief port wait queue */
};

/*! \brief Structure to handle an AXIOM hardware RAW RX ring */
struct axiomnet_raw_rx_hwring {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */
    struct axiomnet_raw_queue sw_queue; /*!< \brief AXIOM software queue */
    /*!< \brief ports of this ring */
    struct axiomnet_sw_port ports[AXIOM_PORT_MAX];
};

/*! \brief Structure to handle an AXIOM hardware RAW TX ring */
struct axiomnet_raw_tx_hwring {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */
    /*!< \brief port of this ring, the TX ring has only 1 port */
    struct axiomnet_sw_port port;
};

/*! \brief Structure to handle an AXIOM hardware RDMA TX ring */
struct axiomnet_rdma_tx_hwring {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */
    struct axiomnet_rdma_queue rdma_queue;/*!< \brief AXIOM software RDMA queue */
    struct axiomnet_long_queue long_queue; /*!< \brief AXIOM software LONG queue */
    /*!< \brief port of this ring to handle RDMA TX messages */
    struct axiomnet_sw_port rdma_port;
    /*!< \brief port of this ring to handle LONG TX messages */
    struct axiomnet_sw_port long_port;
};

/*! \brief Structure to handle an AXIOM hardware RDMA RX ring */
struct axiomnet_rdma_rx_hwring {
    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */
    struct axiomnet_rdma_queue *tx_rdma_queue;
    struct axiomnet_long_queue long_queue; /*!< \brief AXIOM software queue */
    /*!< \brief ports of this ring for LONG messages*/
    struct axiomnet_sw_port long_ports[AXIOM_PORT_MAX];
    //struct axiomnet_rdma_queue sw_queue; /*!< \brief AXIOM software queue */
    /*!< \brief ports of this ring */
    //struct axiomnet_sw_port ports[AXIOM_PORT_MAX];
};

/*! \brief Structure to handle lookup table for LONG buffers */
struct axiomnet_long_buf_lut {
    int buf_id;                         /*!< \brief buffer id*/
    axiomreg_long_buf_t long_buf_hw;    /*!< \brief buffer HW arguments */
    void *long_buf_sw;                  /*!< \brief pointer in the virtual mem*/
};

/*! \brief AXIOM device driver data */
struct axiomnet_drvdata {
    struct device *dev;                 /*!< \brief parent device */
    axiom_dev_t *dev_api;               /*!< \brief AXIOM dev HW API*/
    int devnum;                         /*!< \brief CharDev minor number */
    struct mutex lock;                  /*!< \brief Axiom driver mutex */
    int used;                           /*!< \brief Current number of open() */
    uint8_t port_used;                  /*!< \brief Current port bound */

    /* I/O registers */
    void __iomem *vregs;                /*!< \brief Memory mapped IO registers:
                                                    virtual kernel address */
    struct resource *regs_res;          /*!< \brief IO resource */

    /* IRQ */
    int irq;                            /*!< \brief IRQ descriptor */

    /* DMA */
    void *dma_vaddr;
    dma_addr_t dma_paddr;
    uint64_t dma_size;

    /* RDMA */
    dma_addr_t rdma_paddr;
    uint64_t rdma_size;

    /* LONG */
    void *long_rx_vaddr;
    void *long_tx_vaddr;
    uint64_t long_size;
    struct axiomnet_long_buf_lut long_rxbuf_lut[AXIOMREG_LEN_LONG_BUF];

    /* hardware ring */
    struct axiomnet_raw_tx_hwring raw_tx_ring;  /*!\brief RAW TX ring */
    struct axiomnet_raw_rx_hwring raw_rx_ring;  /*!\brief RAW RX ring */
    struct axiomnet_rdma_tx_hwring rdma_tx_ring;/*!\brief RDMA TX ring */
    struct axiomnet_rdma_rx_hwring rdma_rx_ring;/*!\brief RDMA RX ring */

    /* kthread */
    struct axiom_kthread kthread_raw; /*!< \brief kthread for RAW */
    struct axiom_kthread kthread_rdma; /*!< \brief kthread for RDMA */
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
