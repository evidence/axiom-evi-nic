#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "axiom_user_api.h"
#include "axiom_netdev_user.h"

#define PDEBUG
#include "dprintf.h"

#define AXIOM_DEV_FILENAME      "/dev/axiom0"

typedef struct axiom_dev {
    int fd;

} axiom_dev_t;



axiom_dev_t *
axiom_open(void) {
    axiom_dev_t *dev;

    dev = malloc(sizeof(*dev));
    if (!dev)
        return NULL;

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



axiom_msg_id_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t src_node_id,
        axiom_node_id_t dst_node_id, axiom_raw_type_t type, axiom_data_t data)
{

    return 0;
}

axiom_msg_id_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_node_id_t *dst_node_id, axiom_raw_type_t *type, axiom_data_t *data)
{

    return 0;
}

uint32_t
axiom_read_ni_status(axiom_dev_t *dev)
{
    int ret;
    uint32_t status;

    if (!dev || dev->fd <= 0)
        return -1;

    ret = ioctl(dev->fd, AXNET_GET_STATUS, &status);

    return status;
}

void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    int ret;

    if (!dev || dev->fd <= 0)
        return;

    ret = ioctl(dev->fd, AXNET_SET_CONTROL, &reg_mask);
}

uint32_t
axiom_read_ni_control(axiom_dev_t *dev)
{
    int ret;
    uint32_t control;

    if (!dev || dev->fd <= 0)
        return -1;

    ret = ioctl(dev->fd, AXNET_GET_CONTROL, &control);

    return control;
}


void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    int ret;

    if (!dev || dev->fd <= 0)
        return;

    ret = ioctl(dev->fd, AXNET_SET_NODEID, &node_id);
}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    int ret;
    axiom_node_id_t node_id;

    if (!dev || dev->fd <= 0)
        return -1;

    ret = ioctl(dev->fd, AXNET_GET_NODEID, &node_id);

    return node_id;
}

axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd <= 0)
        return -1;

    routing.node_id = node_id;
    routing.enabled_mask = enabled_mask;

    ret = ioctl(dev->fd, AXNET_SET_ROUTING, &routing);

    return ret;
}

axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd <= 0)
        return -1;

    routing.node_id = node_id;
    routing.enabled_mask = 0;

    ret = ioctl(dev->fd, AXNET_GET_ROUTING, &routing);

    *enabled_mask = routing.enabled_mask;

    return ret;
}

axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    int ret;

    if (!dev || dev->fd <= 0)
        return -1;

    ret = ioctl(dev->fd, AXNET_GET_IFNUMBER, if_number);

    return ret;
}

axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    int ret;
    uint8_t buf_if = if_number;

    if (!dev || dev->fd <= 0)
        return -1;

    ret = ioctl(dev->fd, AXNET_GET_IFINFO, &buf_if);

    *if_features = buf_if;

    return ret;
}

axiom_msg_id_t
axiom_send_raw_neighbour(axiom_dev_t *dev, axiom_if_id_t src_interface,
        axiom_raw_type_t type, axiom_data_t data)
{

    return 0;
}

axiom_msg_id_t
axiom_recv_raw_neighbour(axiom_dev_t *dev, axiom_if_id_t *src_interface,
        axiom_if_id_t *dst_interface, axiom_raw_type_t *type,
        axiom_data_t *data)
{

    return 0;
}
