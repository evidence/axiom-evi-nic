
/*
 * Default implementation of axiom switch logic
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <netinet/in.h>

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_nic_packets.h"

#ifdef AXTP_EXAMPLE1
axiom_topology_t start_topology = {
    .topology = {
        { 1, 2, 3, AXTP_NULL_NODE},
        { 0, 6, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 3, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 4, 2, AXTP_NULL_NODE},
        { 3, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 6, 4, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 5, 7, AXTP_NULL_NODE},
        { 6, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif
#ifdef AXTP_EXAMPLE2
axiom_topology_t start_topology = {
    .topology = {
        { 1, 2, 3, AXTP_NULL_NODE},
        { 0, 4, 8, AXTP_NULL_NODE},
        { 5, 0, 4, AXTP_NULL_NODE},
        { 0, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 2, 1, 5, AXTP_NULL_NODE},
        { 2, 4, 6, 7},
        { 8, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 5, 9, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 6, 1, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 7, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif
#ifdef AXTP_EXAMPLE3
axiom_topology_t start_topology = {
    .topology = {
        { 1, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 3, 3, 4},
        { 0, 4, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 1, 4, 5},
        { 3, 1, 2, AXTP_NULL_NODE},
        { 3, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif
#ifdef AXTP_EXAMPLE4
axiom_topology_t start_topology = {
    .topology = {
        { 1, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 4, 3, 3},
        { 4, 0, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 4, 1, 1, 5},
        { 3, 1, 2, AXTP_NULL_NODE},
        { 3, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif
#ifdef AXTP_EXAMPLE5
axiom_topology_t start_topology = {
    .topology = {
        { 1, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 3, 4, AXTP_NULL_NODE},
        { 2, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 2, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 4, 0, AXTP_NULL_NODE, AXTP_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif

/* given the received socket and messages, return the receiver socket of the
 * neighbour message */
static int
axsw_forward_neighbour(axsw_logic_t *logic, axiom_raw_eth_t *neighbour_msg,
        int src_sd)
{
    int src_vm, dst_vm, dst_sd;
    uint8_t dst_if;

    /* get the index of src virtual machine */
    src_vm = axsw_logic_find_vm_index(logic, src_sd);
    if (src_vm < 0) {
        return -1;
    }

    /* find the receiver virtual machine */
    dst_if = neighbour_msg->raw_msg.header.neighbour.dst_if;
    dst_vm = start_topology.topology[src_vm][dst_if];

    DPRINTF("src_vm_index: %d dst_if: %d dst_vm: %d", src_vm, dst_if, dst_vm);

    /* find receiver socket */
    dst_sd = axsw_logic_find_neighbour_sd(logic, dst_vm);
    if (dst_sd < 0) {
        return -1;
    }

    /* capture AXIOM_DSCV_TYPE_SETID messages in order to memorize socket
     * descriptor associated to each node id for raw messages forwarding */
    if (neighbour_msg->raw_msg.header.neighbour.type == AXIOM_DSCV_TYPE_SETID) {
        axiom_discovery_msg_t *disc_msg;

        disc_msg = (axiom_discovery_msg_t *)
                    (&(neighbour_msg->raw_msg));

        logic->node_sd[disc_msg->data.disc.src_node] = src_sd;
        logic->node_sd[disc_msg->data.disc.dst_node] = dst_sd;

        DPRINTF("catched AXIOM_DSCV_TYPE_REQ_ID - src_node: %d dst_node: %d",
                disc_msg->data.disc.src_node, disc_msg->data.disc.dst_node);
    }

    return dst_sd;
}


/* given the received messages, return the recipient socket of the raw message */
static int
axsw_forward_raw(axsw_logic_t *logic, axiom_raw_eth_t *raw_msg)
{
    uint8_t dst_node;

    dst_node = raw_msg->raw_msg.header.raw.dst_node;

    DPRINTF("dst_node: %d", dst_node);

    return axsw_logic_find_raw_sd(logic, dst_node);
}


int
axsw_logic_forward(axsw_logic_t *logic, int src_sd, axiom_raw_eth_t *axiom_packet)
{
    int dst_sd;

    if (ntohs(axiom_packet->eth_hdr.type) != AXIOM_ETH_TYPE_RAW) {
        EPRINTF("Received a ethernet packet with wrong type");
        return -1;
    }

    DPRINTF("src_sd: %d", src_sd);

    if (axiom_packet->raw_msg.header.raw.flags & AXIOM_RAW_FLAG_NEIGHBOUR) {
        /* neighbour message */
        dst_sd = axsw_forward_neighbour(logic, axiom_packet, src_sd);
    } else {
        /* raw message */
        dst_sd = axsw_forward_raw(logic, axiom_packet);
    }

    DPRINTF("dst_sd: %d", dst_sd);

    if (dst_sd < 0) {
        EPRINTF("dstination socket not found - dst_sd: %d", dst_sd);
        return -1;
    }

    return dst_sd;
}
