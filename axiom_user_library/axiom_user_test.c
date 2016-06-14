/*!
 * \file axiom_user_test.c
 *
 * \version     v0.6
 * \date        2016-05-03
 *
 * This file contains some tests of Axiom NIC API for the user-space
 */
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "dprintf.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_packets.h"

int verbose = 1;

#define AX_TEST_NODEID          33

int main (int argc, char *argv[])
{
    int i;
    axiom_dev_t *dev;
    axiom_node_id_t node_id;
    axiom_port_t port;
    axiom_type_t type;
    axiom_if_id_t if_number;
    uint8_t if_features, enabled_mask;
    uint32_t status, control, payload;
    axiom_payload_size_t payload_size;
    axiom_err_t err;

    dev = axiom_open(NULL);
    if (!dev) {
        EPRINTF("axiom_open failed! - errno = %d", errno);
        return -1;
    }

    status = axiom_read_ni_status(dev);
    IPRINTF(verbose, "read status = 0x%x", status);

    control = axiom_read_ni_control(dev);
    IPRINTF(verbose, "read control = 0x%x", control);

    axiom_set_ni_control(dev, 0x00000001);
    IPRINTF(verbose, "set control = 0x%x", 0x00000001);

    control = axiom_read_ni_control(dev);
    IPRINTF(verbose, "read control = 0x%x", control);

    node_id = axiom_get_node_id(dev);
    IPRINTF(verbose, "startup node_id = 0x%x", node_id);

    axiom_set_node_id(dev, AX_TEST_NODEID);
    IPRINTF(verbose, "set node_id = 0x%x", AX_TEST_NODEID);

    node_id = axiom_get_node_id(dev);
    IPRINTF(verbose, "get node_id = 0x%x", node_id);

    err = axiom_get_if_number(dev, &if_number);
    IPRINTF(verbose, "get if_number = 0x%x - err = %d", if_number, err);

    for (i = 0; i < 4; i++) {
        err = axiom_get_if_info(dev, i, &if_features);
        IPRINTF(verbose, "get if_info[%d] = 0x%x - err = %d - errno = %d",
                i, if_features, err, errno);
    }


    err = axiom_set_routing(dev, 3, 0x12);
    IPRINTF(verbose, "set routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 3, 0x12, err);

    err = axiom_get_routing(dev, 3, &enabled_mask);
    IPRINTF(verbose, "get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 3, enabled_mask, err);

    err = axiom_get_routing(dev, 102, &enabled_mask);
    IPRINTF(verbose, "get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, enabled_mask, err);

    err = axiom_set_routing(dev, 102, 0x33);
    IPRINTF(verbose, "set routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, 0x33, err);

    err = axiom_get_routing(dev, 102, &enabled_mask);
    IPRINTF(verbose, "get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, enabled_mask, err);

    payload = 1234567;

    /* loopback */
    err = axiom_send_raw(dev, 22, 1, AXIOM_TYPE_RAW_DATA, sizeof(payload),
            &payload);
    IPRINTF(verbose, "send raw nodeid = 0x%x port = 0x%x type = 0x%x payload = 0x%x", 22, 1, 0, payload);

    payload_size = sizeof(payload);
    err = axiom_recv_raw(dev, &node_id, &port, &type, &payload_size, &payload);
    IPRINTF(verbose, "recv raw nodeid = 0x%x port = 0x%x type = 0x%x payload = 0x%x", node_id, port, type, payload);

    axiom_set_ni_control(dev, 0x00000000);
    err = axiom_send_raw(dev, 22, 1, AXIOM_TYPE_RAW_DATA, sizeof(payload),
            &payload);
    IPRINTF(verbose, "send raw nodeid = 0x%x port = 0x%x type = 0x%x payload = 0x%x", 22, 1, 0, payload);


    axiom_close(dev);

    return 0;
}
