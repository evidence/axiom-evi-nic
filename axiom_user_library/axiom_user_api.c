/*!
 * \file axiom_user_api.c
 *
 * \version     v0.6
 * \date        2016-05-03
 *
 * This file contains the implementation of Axiom NIC API for the user-space
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "dprintf.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_packets.h"
#include "axiom_netdev_user.h"
#include "axiom_utility.h"

/*! \brief Axiom char dev default name */
#define AXIOM_DEV_FILENAME      "/dev/axiom0"

/*!
 * \brief axiom arguments for the axiom_open() function
 */
typedef struct axiom_dev {
    int fd;             /*!< \brief file descriptor of the AXIOM char dev */
    axiom_flags_t flags;/*!< \brief axiom flags */
    void *rdma_addr;    /*!< \brief rdma zone pointer */
    uint64_t rdma_size; /*!< \brief rdma zone size */
} axiom_dev_t;



axiom_dev_t *
axiom_open(axiom_args_t *args) {
    axiom_dev_t *dev;

    dev = calloc(1, sizeof(*dev));
    if (!dev) {
        EPRINTF("failed to allocate memory");
        return NULL;
    }

    dev->fd = open(AXIOM_DEV_FILENAME, O_RDWR);
    if (dev->fd <0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_FILENAME);
        goto free_dev;
    }

    if (args) {
        axiom_err_t ret;

        ret = axiom_set_flags(dev, args->flags);
        if (ret) {
            EPRINTF("impossible to set flags");
            goto free_dev;
        }
    }

    return dev;

free_dev:
    free(dev);
    return NULL;
}

void
axiom_close(axiom_dev_t *dev)
{
    if (!dev)
        return;
    close(dev->fd);
    free(dev);
}

static axiom_err_t
axiom_update_flags(axiom_dev_t *dev, axiom_flags_t update_flags)
{
    /* no-blocking flag updated */
    if (update_flags & AXIOM_FLAG_NOBLOCK) {
        int fd_flags;

        fd_flags = fcntl(dev->fd, F_GETFL);
        if (fd_flags == -1) {
            EPRINTF("fcntl error - errno: %s", strerror(errno));
            return AXIOM_RET_ERROR;
        }

        if (dev->flags & AXIOM_FLAG_NOBLOCK)
            fd_flags |= O_NONBLOCK;
        else
            fd_flags &= ~O_NONBLOCK;

        if (fcntl(dev->fd, F_SETFL, fd_flags) == -1) {
            EPRINTF("fcntl error - errno: %s", strerror(errno));
            return AXIOM_RET_ERROR;
        }
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_set_flags(axiom_dev_t *dev, axiom_flags_t flags)
{
    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    dev->flags |= flags;

    return axiom_update_flags(dev, flags);
}

axiom_err_t
axiom_unset_flags(axiom_dev_t *dev, axiom_flags_t flags)
{
    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    dev->flags &= ~flags;

    return axiom_update_flags(dev, flags);
}

axiom_err_t
axiom_bind(axiom_dev_t *dev, axiom_port_t port)
{
    axiom_ioctl_bind_t ioctl_bind;
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    ioctl_bind.port = port;
    ioctl_bind.flush = (dev->flags & AXIOM_FLAG_NOFLUSH) ? 0 : 1;

    ret = ioctl(dev->fd, AXNET_BIND, &ioctl_bind);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_next_hop(axiom_dev_t *dev, axiom_node_id_t dst_id,
               axiom_if_id_t *if_number) {
    axiom_err_t ret;
    uint8_t enabled_mask;
    int i;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    ret = axiom_get_routing(dev, dst_id, &enabled_mask);
    if (ret != AXIOM_RET_OK)
        return ret;

    for (i = 0; i < AXIOM_INTERFACES_MAX; i++) {
        if (enabled_mask & (uint8_t)(1 << i)) {
            *if_number = i;
            return AXIOM_RET_OK;
        }
    }
    return AXIOM_RET_ERROR;
}

axiom_err_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_raw_payload_size_t payload_size,
        void *payload)
{
    axiom_ioctl_raw_t raw_msg;
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    if (unlikely(payload_size > AXIOM_RAW_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    raw_msg.header.tx.port_type.field.port = (port & 0x7);
    raw_msg.header.tx.port_type.field.type = (type & 0x7);
    raw_msg.header.tx.dst = dst_id;
    raw_msg.header.tx.payload_size = payload_size;
    raw_msg.payload = payload;

    ret = ioctl(dev->fd, AXNET_SEND_RAW, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN)
            return AXIOM_RET_NOTAVAIL;
        if (errno == EINTR)
            return AXIOM_RET_INTR;
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    DPRINTF("dst: 0x%x payload_size: 0x%x", raw_msg.header.tx.dst,
            raw_msg.header.tx.payload_size);

    return ret;
}

axiom_err_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_type_t *type,
        axiom_raw_payload_size_t *payload_size, void *payload)
{
    axiom_ioctl_raw_t raw_msg;
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    if (unlikely(*payload_size > AXIOM_RAW_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    raw_msg.header.rx.payload_size = *payload_size;
    raw_msg.payload = payload;

    ret = ioctl(dev->fd, AXNET_RECV_RAW, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN)
            return AXIOM_RET_NOTAVAIL;
        if (errno == EINTR)
            return AXIOM_RET_INTR;
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    *src_id = raw_msg.header.rx.src;
    *port = (raw_msg.header.rx.port_type.field.port & 0x7);
    *type = (raw_msg.header.rx.port_type.field.type & 0x7);
    *payload_size = raw_msg.header.rx.payload_size;

    return raw_msg.header.rx.msg_id;
}

int
axiom_send_raw_avail(axiom_dev_t *dev)
{
    int ret;
    int avail;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_SEND_RAW_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return -1;
    }

    return avail;
}

int
axiom_recv_raw_avail(axiom_dev_t *dev)
{
    int ret;
    int avail;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_RECV_RAW_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return -1;
    }

    return avail;
}

axiom_err_t
axiom_flush_raw(axiom_dev_t *dev)
{
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_FLUSH_RAW);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

#define AXIOM_RDMA_FIXED_ADDR           ((void *)(0x30000000))
void *
axiom_rdma_mmap(axiom_dev_t *dev, uint64_t *size)
{
    void *addr;
    int ret;

    if (unlikely(!dev || dev->fd <= 0 || !size)) {
        EPRINTF("axiom device is not opened");
        return NULL;
    }

    if (unlikely(dev->rdma_addr)) {
        EPRINTF("axiom rdma zone already mapped");
        return NULL;
    }

    ret = ioctl(dev->fd, AXNET_RDMA_SIZE, size);
    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return NULL;
    }

    if (unlikely(*size <= 0)) {
        EPRINTF("size error [%" PRIu64 "]", *size);
        return NULL;
    }

    addr = mmap(AXIOM_RDMA_FIXED_ADDR, *size, PROT_READ | PROT_WRITE,
            MAP_FIXED | MAP_SHARED, dev->fd, 0);
    if (unlikely(addr == MAP_FAILED)) {
        EPRINTF("mmap failed - addr: %p - errno: %s", addr, strerror(errno));
        return NULL;
    }

    dev->rdma_addr = addr;
    dev->rdma_size = *size;

    return addr;
}

axiom_err_t
axiom_rdma_munmap(axiom_dev_t *dev)
{
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    if (unlikely(!dev->rdma_addr)) {
        EPRINTF("axiom rdma zone not mapped");
        return AXIOM_RET_ERROR;
    }

    ret = munmap(dev->rdma_addr, dev->rdma_size);
    if (unlikely(ret)) {
        EPRINTF("munmap failed - errno: %s", strerror(errno));
        return AXIOM_RET_ERROR;
    }

    dev->rdma_addr = NULL;
    dev->rdma_size = 0;

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t remote_id,
        axiom_port_t port, axiom_rdma_payload_size_t payload_size,
        axiom_addr_t local_src_addr, axiom_addr_t remote_dst_addr)
{
    axiom_rdma_hdr_t rdma_hdr;
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    if (unlikely(payload_size >
            (AXIOM_RDMA_PAYLOAD_MAX_SIZE >> AXIOM_RDMA_PAYLOAD_SIZE_ORDER))) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    DPRINTF("[packet] payload_size: 0x%x", payload_size);

    rdma_hdr.tx.port_type.field.type = AXIOM_TYPE_RDMA_WRITE;
    rdma_hdr.tx.port_type.field.port = port;
    rdma_hdr.tx.port_type.field.s = 0;
    rdma_hdr.tx.dst = remote_id;
    rdma_hdr.tx.payload_size = payload_size;
    rdma_hdr.tx.src_addr = local_src_addr;
    rdma_hdr.tx.dst_addr = remote_dst_addr;


    ret = ioctl(dev->fd, AXNET_RDMA_WRITE, &rdma_hdr);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN)
            return AXIOM_RET_NOTAVAIL;
        if (errno == EINTR)
            return AXIOM_RET_INTR;
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return ret;
}

axiom_err_t
axiom_rdma_read(axiom_dev_t *dev, axiom_node_id_t remote_id,
        axiom_port_t port, axiom_rdma_payload_size_t payload_size,
        axiom_addr_t remote_src_addr, axiom_addr_t local_dst_addr)
{
    axiom_rdma_hdr_t rdma_hdr;
    int ret;

    if (unlikely(!dev || dev->fd <= 0)) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    if (unlikely(payload_size >
            (AXIOM_RDMA_PAYLOAD_MAX_SIZE >> AXIOM_RDMA_PAYLOAD_SIZE_ORDER))) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    rdma_hdr.tx.port_type.field.type = AXIOM_TYPE_RDMA_READ;
    rdma_hdr.tx.port_type.field.port = port;
    rdma_hdr.tx.port_type.field.s = 0;
    rdma_hdr.tx.dst = remote_id;
    rdma_hdr.tx.payload_size = payload_size;
    rdma_hdr.tx.src_addr = remote_src_addr;
    rdma_hdr.tx.dst_addr = local_dst_addr;


    ret = ioctl(dev->fd, AXNET_RDMA_READ, &rdma_hdr);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN)
            return AXIOM_RET_NOTAVAIL;
        if (errno == EINTR)
            return AXIOM_RET_INTR;
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return ret;
}

uint32_t
axiom_read_ni_status(axiom_dev_t *dev)
{
    int ret;
    uint32_t status;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_STATUS, &status);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return status;
}

void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return;
    }

    ret = ioctl(dev->fd, AXNET_SET_CONTROL, &reg_mask);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }
}

uint32_t
axiom_read_ni_control(axiom_dev_t *dev)
{
    int ret;
    uint32_t control;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_CONTROL, &control);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return control;
}


void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return;
    }

    ret = ioctl(dev->fd, AXNET_SET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }
}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    int ret;
    axiom_node_id_t node_id;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return node_id;
}

axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = enabled_mask;

    ret = ioctl(dev->fd, AXNET_SET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = 0;

    ret = ioctl(dev->fd, AXNET_GET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    *enabled_mask = routing.enabled_mask;

    return AXIOM_RET_OK;
}

int
axiom_get_num_nodes(axiom_dev_t *dev)
{
    axiom_err_t err;
    uint8_t enabled_mask;
    int i;
    /* init to 1, because the local node is not set in the routing table */
    int num_nodes = 1;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return -1;
    }

    for (i = 0; i < AXIOM_NODES_MAX; i++) {
        err = axiom_get_routing(dev, i, &enabled_mask);
        if (err)
            break;

        /* count node i, if it is reachable through some interface */
        if (enabled_mask)
            num_nodes++;
    }

    return num_nodes;
}


axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_GET_IFNUMBER, if_number);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    int ret;
    uint8_t buf_if = if_number;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device is not opened");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_GET_IFINFO, &buf_if);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    *if_features = buf_if;

    return AXIOM_RET_OK;
}
