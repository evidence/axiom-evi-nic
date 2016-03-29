#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "axiom_user_api.h"
#define PDEBUG
#include "dprintf.h"


#define AX_TEST_NODEID          33

int main (int argc, char *argv[])
{
    int i;
    axiom_dev_t *dev;
    axiom_node_id_t node_id;
    axiom_if_id_t if_number;
    uint8_t if_features, enabled_mask;
    uint32_t status, control;
    axiom_err_t err;

    dev = axiom_open(NULL);
    if (!dev) {
        EPRINTF("axiom_open failed! - errno = %d", errno);
        return -1;
    }

    status = axiom_read_ni_status(dev);
    IPRINTF("read status = 0x%x", status);

    control = axiom_read_ni_control(dev);
    IPRINTF("read control = 0x%x", control);

    axiom_set_ni_control(dev, 0x00000001);
    IPRINTF("set control = 0x%x", 0x00000001);

    control = axiom_read_ni_control(dev);
    IPRINTF("read control = 0x%x", control);

    node_id = axiom_get_node_id(dev);
    IPRINTF("startup node_id = 0x%x", node_id);

    axiom_set_node_id(dev, AX_TEST_NODEID);
    IPRINTF("set node_id = 0x%x", AX_TEST_NODEID);

    node_id = axiom_get_node_id(dev);
    IPRINTF("get node_id = 0x%x", node_id);

    err = axiom_get_if_number(dev, &if_number);
    IPRINTF("get if_number = 0x%x - err = %d", if_number, err);

    for (i = 0; i < 4; i++) {
        err = axiom_get_if_info(dev, i, &if_features);
        IPRINTF("get if_info[%d] = 0x%x - err = %d - errno = %d",
                i, if_features, err, errno);
    }


    err = axiom_set_routing(dev, 3, 0x12);
    IPRINTF("set routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 3, 0x12, err);

    err = axiom_get_routing(dev, 3, &enabled_mask);
    IPRINTF("get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 3, enabled_mask, err);

    err = axiom_get_routing(dev, 102, &enabled_mask);
    IPRINTF("get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, enabled_mask, err);

    err = axiom_set_routing(dev, 102, 0x33);
    IPRINTF("set routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, 0x33, err);

    err = axiom_get_routing(dev, 102, &enabled_mask);
    IPRINTF("get routing node_id = 0x%x  enabled_mask = 0x%x - err = %d", 102, enabled_mask, err);

    axiom_close(dev);

    return 0;
}
