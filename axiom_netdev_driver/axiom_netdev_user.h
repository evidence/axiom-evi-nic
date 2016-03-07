#ifndef AXIOM_NETDEV_USER_h
#define AXIOM_NETDEV_USER_h

#define AXIOM_MSG_TYPE_RAW      0
#define AXIOM_MSG_TYPE_RDMA     1
#if 0
typedef struct axiom_msg {
    uint8_t type;
    uint8_t spare[3];
    union {
        axiom_raw_msg_t raw_msg;
        axiom_RDMA_msg_t rdma_msg;
    } payload;

} axiom_msg_t;
#endif
typedef struct axiom_ioctl_routing {
    uint8_t node_id;
    uint8_t enabled_mask;
} axiom_ioctl_routing_t;

/* ioctl defines */
#define AXNET_MAGIC  0xAA
#define AXNET_SET_NODEID        _IOW(AXNET_MAGIC, 100, int)
#define AXNET_GET_NODEID        _IOR(AXNET_MAGIC, 101, int)
#define AXNET_SET_ROUTING       _IOW(AXNET_MAGIC, 102, axiom_ioctl_routing_t)
#define AXNET_GET_ROUTING       _IOR(AXNET_MAGIC, 103, axiom_ioctl_routing_t)
#define AXNET_GET_IFNUMBER      _IOR(AXNET_MAGIC, 104, uint8_t)
#define AXNET_GET_IFINFO        _IOWR(AXNET_MAGIC, 104, uint8_t)


#endif /* !AXIOM_NETDEV_USER_h */
