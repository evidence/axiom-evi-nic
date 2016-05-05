#ifndef AXIOM_NETDEV_USER_h
#define AXIOM_NETDEV_USER_h
/*!
 * \file axiom_netdev_user.h
 *
 * \version     v0.4
 * \date        2016-05-03
 *
 * This file contains the Axiom-NIC interface beetween user-space and kernel
 * through IOCTLs and write()/read()
 */

/*! \brief AXIOM message type SMALL */
#define AXIOM_MSG_TYPE_SMALL    0
/*! \brief AXIOM message type RDMA */
#define AXIOM_MSG_TYPE_RDMA     1

#if 0
typedef struct axiom_msg {
    uint8_t type;
    uint8_t spare[3];
    union {
        axiom_small_msg_t small_msg;
        axiom_RDMA_msg_t rdma_msg;
    } payload;

} axiom_msg_t;
#endif

/*! \brief AXIOM ioctl routing parameters */
typedef struct axiom_ioctl_routing {
    uint8_t node_id;            /*!< \brief node identifier to reach */
    uint8_t enabled_mask;       /*!< \brief mask of interface enabled */
} axiom_ioctl_routing_t;

/* ioctl defines */

/*! \brief AXIOM IOCTL magic number used in the IOCTL id*/
#define AXNET_MAGIC  0xAA
/*! \brief AXIOM IOCTL to set local node id */
#define AXNET_SET_NODEID        _IOW(AXNET_MAGIC, 100, uint8_t)
/*! \brief AXIOM IOCTL to get local node id */
#define AXNET_GET_NODEID        _IOR(AXNET_MAGIC, 101, uint8_t)
/*! \brief AXIOM IOCTL to set a row in the local routing table */
#define AXNET_SET_ROUTING       _IOW(AXNET_MAGIC, 102, axiom_ioctl_routing_t)
/*! \brief AXIOM IOCTL to get a row from the local routing table */
#define AXNET_GET_ROUTING       _IOR(AXNET_MAGIC, 103, axiom_ioctl_routing_t)
/*! \brief AXIOM IOCTL to get the number of local interfaces */
#define AXNET_GET_IFNUMBER      _IOR(AXNET_MAGIC, 104, uint8_t)
/*! \brief AXIOM IOCTL to get the info about local interface */
#define AXNET_GET_IFINFO        _IOWR(AXNET_MAGIC, 105, uint8_t)
/*! \brief AXIOM IOCTL to get status register value */
#define AXNET_GET_STATUS        _IOR(AXNET_MAGIC, 106, uint32_t)
/*! \brief AXIOM IOCTL to set control register value */
#define AXNET_SET_CONTROL       _IOW(AXNET_MAGIC, 107, uint32_t)
/*! \brief AXIOM IOCTL to get control register value */
#define AXNET_GET_CONTROL       _IOR(AXNET_MAGIC, 108, uint32_t)
/*! \brief AXIOM IOCTL to bind a process on a specified port */
#define AXNET_BIND              _IOW(AXNET_MAGIC, 109, uint8_t)


#endif /* !AXIOM_NETDEV_USER_h */
