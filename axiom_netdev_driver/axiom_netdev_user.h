#ifndef AXIOM_NETDEV_USER_h
#define AXIOM_NETDEV_USER_h
/*!
 * \file axiom_netdev_user.h
 *
 * \version     v0.9
 * \date        2016-06-22
 *
 * This file contains the Axiom-NIC interface beetween user-space and kernel
 * through IOCTLs
 */

/*! \brief AXIOM ioctl routing parameters */
typedef struct axiom_ioctl_routing {
    uint8_t node_id;            /*!< \brief node identifier to reach */
    uint8_t enabled_mask;       /*!< \brief mask of interface enabled */
} axiom_ioctl_routing_t;

/*! \brief AXIOM ioctl RAW messages descriptor with a pointer to the payload */
typedef struct axiom_ioctl_raw {
    axiom_raw_hdr_t header;     /*!< \brief message header */
    void *payload;              /*!< \brief pointer to the message payload */
} axiom_ioctl_raw_t;

/*! \brief AXIOM ioctl RAW messages descriptor with iovec for the payload */
typedef struct axiom_ioctl_raw_iov {
    axiom_raw_hdr_t header;     /*!< \brief message header */
    struct iovec *iov;          /*!< \brief iovec array */
    int iovcnt;                 /*!< \brief iovec counter */
} axiom_ioctl_raw_iov_t;

/*! \brief AXIOM ioctl LONG messages descriptor with iovec for the payload */
typedef struct axiom_ioctl_long_iov {
    axiom_rdma_hdr_t header;    /*!< \brief message header */
    struct iovec *iov;          /*!< \brief iovec array */
    int iovcnt;                 /*!< \brief iovec counter */
} axiom_ioctl_long_iov_t;

/*! \brief AXIOM ioctl bind parameters */
typedef struct axiom_ioctl_bind {
    uint8_t port;               /*!< \brief port to bind */
    uint8_t flush;              /*!< \brief flush previous packets */
} axiom_ioctl_bind_t;

/*! \brief AXIOM ioctl RDMA  */
typedef struct axiom_ioctl_rdma {
    axiom_rdma_hdr_t header;    /*!< \brief message header */
    void *src_addr;             /*!< \brief source virtual address */
    void *dst_addr;             /*!< \brief destination virtual address */
    int app_id;                 /*!< \brief application ID */
} axiom_ioctl_rdma_t;


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
#define AXNET_BIND              _IOWR(AXNET_MAGIC, 109, axiom_ioctl_bind_t)
/*! \brief AXIOM IOCTL to send a raw message */
#define AXNET_SEND_RAW          _IOW(AXNET_MAGIC, 110, axiom_ioctl_raw_t)
/*! \brief AXIOM IOCTL to recv a raw message */
#define AXNET_RECV_RAW          _IOWR(AXNET_MAGIC, 111, axiom_ioctl_raw_t)
/*! \brief AXIOM IOCTL to check if it is possible to send a raw message */
#define AXNET_SEND_RAW_AVAIL    _IOW(AXNET_MAGIC, 112, int)
/*! \brief AXIOM IOCTL to check if it is possible to receive a raw message */
#define AXNET_RECV_RAW_AVAIL    _IOWR(AXNET_MAGIC, 113, int)
/*! \brief AXIOM IOCTL to flush raw messages */
#define AXNET_FLUSH_RAW         _IO(AXNET_MAGIC, 114)
/*! \brief AXIOM IOCTL to get the size */
#define AXNET_RDMA_SIZE         _IOR(AXNET_MAGIC, 115, uint64_t)
/*! \brief AXIOM IOCTL to start a RDMA write */
#define AXNET_RDMA_WRITE        _IOW(AXNET_MAGIC, 116, axiom_ioctl_rdma_t)
/*! \brief AXIOM IOCTL to start a RDMA read */
#define AXNET_RDMA_READ         _IOW(AXNET_MAGIC, 117, axiom_ioctl_rdma_t)
/*! \brief AXIOM IOCTL to start a RDMA write */
#define AXNET_SEND_LONG         _IOW(AXNET_MAGIC, 118, axiom_long_msg_t)
/*! \brief AXIOM IOCTL to start a RDMA read */
#define AXNET_RECV_LONG         _IOWR(AXNET_MAGIC, 119, axiom_long_msg_t)
/*! \brief AXIOM IOCTL to check if it is possible to send a long message */
#define AXNET_SEND_LONG_AVAIL   _IOW(AXNET_MAGIC, 120, int)
/*! \brief AXIOM IOCTL to check if it is possible to receive a long message */
#define AXNET_RECV_LONG_AVAIL   _IOWR(AXNET_MAGIC, 121, int)
/*! \brief AXIOM IOCTL to flush long messages */
#define AXNET_FLUSH_LONG        _IO(AXNET_MAGIC, 122)
/*! \brief AXIOM IOCTL to send a raw message with iovec */
#define AXNET_SEND_RAW_IOV      _IOW(AXNET_MAGIC, 123, axiom_ioctl_raw_iov_t)
/*! \brief AXIOM IOCTL to recv a raw message with iovec */
#define AXNET_RECV_RAW_IOV      _IOWR(AXNET_MAGIC, 124, axiom_ioctl_raw_iov_t)
/*! \brief AXIOM IOCTL to send a long message with iovec */
#define AXNET_SEND_LONG_IOV      _IOW(AXNET_MAGIC, 125, axiom_ioctl_long_iov_t)
/*! \brief AXIOM IOCTL to recv a long message with iovec */
#define AXNET_RECV_LONG_IOV      _IOWR(AXNET_MAGIC, 126, axiom_ioctl_long_iov_t)

#define AXNET_DEBUG_INFO        _IO(AXNET_MAGIC, 200)

#endif /* !AXIOM_NETDEV_USER_h */
