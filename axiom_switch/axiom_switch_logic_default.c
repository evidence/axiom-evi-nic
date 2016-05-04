/*!
 * \file axiom_switch_logic_default.c
 *
 * \version     v0.4
 * \date        2016-05-03
 *
 * This file contains the default implementation of the logic in the Axiom Switch
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include <netinet/in.h>

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_nic_packets.h"
#include "axiom_nic_discovery.h"

static int
axsw_forward_neighbour(axsw_logic_t *logic, axiom_small_eth_t *neighbour_msg,
        int src_sd)
{
    int src_vm, dst_vm, dst_sd;
    uint8_t dst_if, neighbour_if;

    /* get the index of src virtual machine */
    src_vm = axsw_logic_find_vm_id(logic, src_sd);
    if (src_vm < 0) {
        return -1;
    }

    /* find the receiver virtual machine */
    dst_if = neighbour_msg->small_msg.header.tx.dst;
    dst_vm = logic->start_topology.topology[src_vm][dst_if];

    NDPRINTF("src_vm_id: %d dst_if: %d dst_vm: %d", src_vm, dst_if, dst_vm);

    /* find receiver socket */
    dst_sd = axsw_logic_find_neighbour_sd(logic, dst_vm);
    if (dst_sd < 0) {
        return -1;
    }

    /* find the recipient receiving interface */
    neighbour_if = axsw_logic_find_neighbour_if(logic, src_vm,
            neighbour_msg->small_msg.header.tx.dst);
    if (neighbour_if < 0) {
        return -1;
    }
    DPRINTF("neighbour_if %d", neighbour_if);
    /* set receiver interface */
    neighbour_msg->small_msg.header.rx.src = neighbour_if;

    /* capture AXIOM_DSCV_CMD_SETID messages in order to memorize socket
     * descriptor associated to each node id for small messages forwarding */
    if (neighbour_msg->small_msg.header.tx.port_flag.field.port
            == AXIOM_SMALL_PORT_INIT) {
        axiom_discovery_payload_t *disc_payload;

        disc_payload = (axiom_discovery_payload_t *)
                    (&(neighbour_msg->small_msg.payload));

        if (disc_payload->command == AXIOM_DSCV_CMD_SETID) {
            logic->node_sd[disc_payload->src_node] = src_sd;
            logic->node_sd[disc_payload->dst_node] = dst_sd;

            DPRINTF("catched AXIOM_DSCV_CMD_SETID - src_node: %d dst_node: %d",
                    disc_payload->src_node, disc_payload->dst_node);
        }
    }

    return dst_sd;
}

static int
axsw_forward_small(axsw_logic_t *logic, axiom_small_eth_t *small_msg,
        int src_sd)
{
    uint8_t dst_node, src_node;

    dst_node = small_msg->small_msg.header.tx.dst;
    src_node = axsw_logic_find_node_id(logic, src_sd);

    DPRINTF("dst_node: %d - src_node: %d", dst_node, src_node);

    /* set source node id in the packet */
    small_msg->small_msg.header.rx.src = src_node;

    return axsw_logic_find_small_sd(logic, dst_node);
}

int
axsw_logic_forward(axsw_logic_t *logic, int src_sd,
        axiom_small_eth_t *axiom_packet)
{
    int dst_sd;

    if (ntohs(axiom_packet->eth_hdr.type) != AXIOM_ETH_TYPE_SMALL) {
        EPRINTF("Received a ethernet packet with wrong type");
        return -1;
    }

    DPRINTF("src_sd: %d", src_sd);

    if (axiom_packet->small_msg.header.tx.port_flag.field.flag &
            AXIOM_SMALL_FLAG_NEIGHBOUR) {
        /* neighbour message */
        dst_sd = axsw_forward_neighbour(logic, axiom_packet, src_sd);
    } else {
        /* small message */
        dst_sd = axsw_forward_small(logic, axiom_packet, src_sd);
    }

    NDPRINTF("dst_sd: %d", dst_sd);

    if (dst_sd < 0) {
        EPRINTF("dstination socket not found - dst_sd: %d", dst_sd);
        return -1;
    }

    return dst_sd;
}
