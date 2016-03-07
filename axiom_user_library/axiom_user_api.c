#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "axiom_user_api.h"

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

void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{

}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    uint32_t ret;


    return ret;
}

axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{

    return 0;
}

axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{

    return 0;
}

axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{

    return 0;
}

axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{

    return 0;
}

axiom_msg_id_t
axiom_send_raw_neighbour(axiom_dev_t *dev, axiom_raw_sub_type_t sub_type,
        axiom_node_id_t src_node_id, axiom_node_id_t dst_node_id,
        axiom_if_id_t src_interface, axiom_if_id_t dst_interface, uint8_t data)
{

    return 0;
}

axiom_msg_id_t
axiom_recv_raw_neighbour (axiom_dev_t *dev, axiom_raw_sub_type_t *sub_type,
        axiom_node_id_t* src_node_id, axiom_node_id_t* dst_node_id,
        axiom_if_id_t* src_interface, axiom_if_id_t* dst_interface,
        uint8_t* data)
{

    return 0;
}
