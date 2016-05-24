/*!
 * \file axiom_user_api.c
 *
 * \version     v0.5
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "dprintf.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_packets.h"
#include "axiom_netdev_user.h"

/*! \brief Axiom char dev default name */
#define AXIOM_DEV_FILENAME      "/dev/axiom0"

/*!
 * \brief axiom arguments for the axiom_open() function
 */
typedef struct axiom_dev {
    int fd; /*!< \brief file descriptor of the AXIOM char dev */
} axiom_dev_t;



axiom_dev_t *
axiom_open(axiom_args_t *args) {
    axiom_dev_t *dev;

    dev = malloc(sizeof(*dev));
    if (!dev) {
        EPRINTF("failed to allocate memory");
        return NULL;
    }

    dev->fd = open(AXIOM_DEV_FILENAME, O_RDWR);
    if (dev->fd <0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_FILENAME);
        goto free_dev;
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

axiom_err_t
axiom_bind(axiom_dev_t *dev, axiom_port_t port)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_BIND, &port);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
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
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    ret = axiom_get_routing(dev, dst_id, &enabled_mask);
    if (ret == AXIOM_RET_ERROR)
        return ret;

    for (i = 0; i < AXIOM_MAX_INTERFACES; i++) {
        if (enabled_mask & (uint8_t)(1 << i)) {
            *if_number = i;
            return AXIOM_RET_OK;
        }
    }
    return AXIOM_RET_ERROR;
}

axiom_msg_id_t
axiom_send_small(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_payload_size_t payload_size,
        void *payload)
{
    axiom_small_msg_t small_msg;
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    if (payload_size > AXIOM_SMALL_PAYLOAD_MAX_SIZE) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_SMALL_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    small_msg.header.tx.port_type.field.port = (port & 0x7);
    small_msg.header.tx.port_type.field.type = (type & 0x7);
    small_msg.header.tx.dst = dst_id;
    small_msg.header.tx.payload_size = payload_size;
    /* TODO: pass to the kernel only the address of the payload */
    memcpy(&small_msg.payload, payload, payload_size);

    /* TODO: maybe we can send only the payload bytes used */
    ret = write(dev->fd, &small_msg, sizeof(small_msg));
    if (ret != sizeof(small_msg)) {
        EPRINTF("impossible to write() - ret: %d", ret);
        return AXIOM_RET_ERROR;
    }

    DPRINTF("dst: %x payload_size: %x", small_msg.header.tx.dst,
            small_msg.header.tx.payload_size);

    return AXIOM_RET_OK;
}

axiom_msg_id_t
axiom_recv_small(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_type_t *type,
        axiom_payload_size_t *payload_size, void *payload)
{
    axiom_small_msg_t small_msg;
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    if (*payload_size > AXIOM_SMALL_PAYLOAD_MAX_SIZE) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_SMALL_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    ret = read(dev->fd, &small_msg, sizeof(small_msg));
    if (ret != sizeof(small_msg))
        return AXIOM_RET_ERROR;

    if (small_msg.header.rx.payload_size > *payload_size) {
        EPRINTF("receive packet bigger[%d] than payload_size[%d] specified",
                small_msg.header.rx.payload_size, *payload_size);
        return AXIOM_RET_ERROR;
    }

    *src_id = small_msg.header.rx.src;
    *port = (small_msg.header.rx.port_type.field.port & 0x7);
    *type = (small_msg.header.rx.port_type.field.type & 0x7);
    *payload_size = small_msg.header.rx.payload_size;
    memcpy(payload, &small_msg.payload, small_msg.header.rx.payload_size);

    return AXIOM_RET_OK;
}

uint32_t
axiom_read_ni_status(axiom_dev_t *dev)
{
    int ret;
    uint32_t status;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_STATUS, &status);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
    }

    return status;
}

void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return;
    }

    ret = ioctl(dev->fd, AXNET_SET_CONTROL, &reg_mask);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
    }
}

uint32_t
axiom_read_ni_control(axiom_dev_t *dev)
{
    int ret;
    uint32_t control;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_CONTROL, &control);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
    }

    return control;
}


void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    int ret;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return;
    }

    ret = ioctl(dev->fd, AXNET_SET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
    }
}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    int ret;
    axiom_node_id_t node_id;

    if (!dev || dev->fd <= 0) {
        EPRINTF("axiom device not open");
        return -1;
    }

    ret = ioctl(dev->fd, AXNET_GET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
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
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = enabled_mask;

    ret = ioctl(dev->fd, AXNET_SET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
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
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = 0;

    ret = ioctl(dev->fd, AXNET_GET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
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
        EPRINTF("axiom device not open");
        return -1;
    }

    for (i = 0; i < AXIOM_MAX_NODES; i++) {
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
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_GET_IFNUMBER, if_number);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
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
        EPRINTF("axiom device not open");
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd, AXNET_GET_IFINFO, &buf_if);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %d", ret, errno);
        return AXIOM_RET_ERROR;
    }

    *if_features = buf_if;

    return AXIOM_RET_OK;
}
