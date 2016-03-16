
/*
 * Default implementation of axiom switch logic
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "axiom_switch_logic.h"
#include "axiom_switch_configuration.h"
#include "axiom_switch_packets.h"
#include "axiom_nic_packets.h"

#include "dprintf.h"

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

/* given the received socket and messages, return the recipient
   socket of the neighbour message */
static int manage_neighbour_msg(axiom_raw_eth_t *neighbour_msg,
                                int *vm_sd, int my_sd, int *dest_sd,
                                int *node_sd)
{
    uint8_t dst_if;
    uint8_t my_vm_index, dest_vm_index;
    int ret = 0;

    /* get the index of my virtual machine */
    ret = find_vm_index(my_sd, vm_sd, &my_vm_index);
    if (ret != 0)
    {
        return ret;
    }

    /* find the receiver virtual machine */
    dst_if = neighbour_msg->raw_msg.header.neighbour.dst_if;
    dest_vm_index = start_topology.topology[my_vm_index][dst_if];

    /* recipient socket */
    find_neighbour_sd(dest_vm_index, vm_sd, dest_sd);

    /* capture AXIOM_DSCV_TYPE_REQ_ID messages in order to
       memorize socket descriptor associated to each node id
       (for raw messages management!) */
    if (neighbour_msg->raw_msg.header.neighbour.type == AXIOM_DSCV_TYPE_REQ_ID)
    {
        axiom_discovery_msg_t *disc_msg;

        disc_msg = (axiom_discovery_msg_t *)
                    (&(neighbour_msg->raw_msg));
        node_sd[disc_msg->data.disc.src_node] = my_sd;
    }

    return ret;
}

/* given the received messages, return the recipient
   socket of the raw message */
static int manage_raw_msg(axiom_raw_eth_t *raw_msg, int *dest_sd,
                          int *node_sd)
{
    int ret = 0;
    uint8_t dst_node;

    dst_node = raw_msg->raw_msg.header.raw.dst_node;

    find_raw_sd(dst_node, node_sd, dest_sd);

    return ret;
}


int
axsw_logic_forward(axsw_logic_t *logic, char *buffer, uint32_t length,
        int my_sd)
{
    axiom_raw_eth_t *axiom_packet;
    int ret, dest_sd;

    if (sizeof(axiom_raw_eth_t) != length)
    {
        printf("Received a ethernet packet with unexpected length");
        return- 1;
    }

    axiom_packet = (axiom_raw_eth_t *)buffer;

    if (axiom_packet->eth_hdr.type != AXIOM_ETH_TYPE_RAW)
    {
        printf("Received a ethernet packet with wrong type");
        return -1;
    }

    if (axiom_packet->raw_msg.header.raw.flags & AXIOM_RAW_FLAG_NEIGHBOUR)
    {
        /* neighbour message */
        ret = manage_neighbour_msg(axiom_packet, logic->vm_sd, my_sd,
                                   &dest_sd, logic->node_sd);
    }
    else
    {
        /* raw message */
        ret = manage_raw_msg(axiom_packet, &dest_sd, logic->node_sd);
    }

    /* TODO: if dest_sd is -1 or 0? */

    if (ret != 0)
    {
        printf("message management error\n");
        return ret;
    }

    ret = send(dest_sd, &length, sizeof(length), 0);
    if (ret != sizeof(length))
    {
        DPRINTF("length send error");
        return -1;
    }

    /* send message to the receiver */
    ret = send(dest_sd, buffer, length, 0);
    if (ret != length)
    {
        DPRINTF("message send error");
        return -1;
    }

    return 0;
}
