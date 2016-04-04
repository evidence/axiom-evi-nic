
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
#include "axiom_nic_discovery.h"

#if 0
#ifdef AXTP_EXAMPLE0
axiom_topology_t start_topology = {
    .topology = {
        { 2, 3, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 3, 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 0, 1, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
        { 1, 0, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
    },
    .num_nodes = AXTP_NUM_NODES,
    .num_interfaces = AXTP_NUM_INTERFACES
};
#endif
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
#endif

/* nodes topology variable */
axiom_topology_t start_topology;

/* given the received socket and messages, return the receiver socket of the
 * neighbour message */
static int
axsw_forward_neighbour(axsw_logic_t *logic, axiom_small_eth_t *neighbour_msg,
        int src_sd)
{
    int src_vm, dst_vm, dst_sd;
    uint8_t dst_if, neighbour_if;

    /* get the index of src virtual machine */
    src_vm = axsw_logic_find_vm_index(logic, src_sd);
    if (src_vm < 0) {
        return -1;
    }

    /* find the receiver virtual machine */
    dst_if = neighbour_msg->small_msg.header.tx.dst;
    dst_vm = start_topology.topology[src_vm][dst_if];

    NDPRINTF("src_vm_index: %d dst_if: %d dst_vm: %d", src_vm, dst_if, dst_vm);

    /* find receiver socket */
    dst_sd = axsw_logic_find_neighbour_sd(logic, dst_vm);
    if (dst_sd < 0) {
        return -1;
    }

    /* find the recipient receiving interface */
    neighbour_if = axsw_logic_find_neighbour_if(&start_topology, src_vm,
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
            == AXIOM_SMALL_PORT_DISCOVERY) {
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


/* given the received messages, return the recipient socket of the small message */
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
axsw_logic_forward(axsw_logic_t *logic, int src_sd, axiom_small_eth_t *axiom_packet)
{
    int dst_sd;

    if (ntohs(axiom_packet->eth_hdr.type) != AXIOM_ETH_TYPE_SMALL) {
        EPRINTF("Received a ethernet packet with wrong type");
        return -1;
    }

    DPRINTF("src_sd: %d", src_sd);

    if (axiom_packet->small_msg.header.tx.port_flag.field.flag & AXIOM_SMALL_FLAG_NEIGHBOUR) {
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

/* functions for toplogy management */
/* Initializes start_toplogy with no connected nodes */
void
axsw_init_topology(axiom_topology_t *start_topology) {
    int i,j;

    for (i = 0; i < AXIOM_MAX_NUM_NODES; i++) {
        for (j = 0; j < AXIOM_NUM_INTERFACES; j++) {
            start_topology->topology[i][j] = AXTP_NULL_NODE;
        }
    }
    start_topology->num_nodes =  AXIOM_MAX_NUM_NODES;
    start_topology->num_interfaces =  AXTP_NUM_INTERFACES;
}

/* Initializes start_toplogy (conf 0) with the connected nodes */
void axsw_init_topology_0(axiom_topology_t *start_topology) {
    /* { 2, 3, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
       { 3, 2, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
       { 0, 1, AXIOM_NULL_NODE, AXIOM_NULL_NODE},
       { 1, 0, AXIOM_NULL_NODE, AXIOM_NULL_NODE}, */
    /* node 0 */
    start_topology->topology[0][0] = 2;
    start_topology->topology[0][1] = 3;
    /* node 1 */
    start_topology->topology[1][0] = 3;
    start_topology->topology[1][1] = 2;
    /* node 2 */
    start_topology->topology[2][0] = 0;
    start_topology->topology[2][1] = 1;
    /* node 3*/
    start_topology->topology[3][0] = 1;
    start_topology->topology[3][1] = 0;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_0;

    printf ("Init topology 0\n");
}

/* Initializes start_toplogy (conf 1) with the connected nodes */
void axsw_init_topology_1(axiom_topology_t *start_topology) {
    /*  { 1, 2, 3, AXTP_NULL_NODE},
        { 0, 6, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 3, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 4, 2, AXTP_NULL_NODE},
        { 3, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 6, 4, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 5, 7, AXTP_NULL_NODE},
        { 6, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE} */
    /* node 0 */
    start_topology->topology[0][0] = 1;
    start_topology->topology[0][1] = 2;
    start_topology->topology[0][2] = 3;
    /* node 1 */
    start_topology->topology[1][0] = 0;
    start_topology->topology[1][1] = 6;
    /* node 2 */
    start_topology->topology[2][0] = 0;
    start_topology->topology[2][1] = 3;
    /* node 3 */
    start_topology->topology[3][0] = 0;
    start_topology->topology[3][1] = 4;
    start_topology->topology[3][2] = 2;
    /* node 4 */
    start_topology->topology[4][0] = 3;
    start_topology->topology[4][1] = 5;
    /* node 5 */
    start_topology->topology[5][0] = 6;
    start_topology->topology[5][1] = 4;
    /* node 6 */
    start_topology->topology[6][0] = 1;
    start_topology->topology[6][1] = 5;
    start_topology->topology[6][2] = 7;
    /* node 7 */
    start_topology->topology[7][0] = 6;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_1;

    printf ("Init topology 1\n");
}

/* Initializes start_toplogy (conf 2) with the connected nodes */
void axsw_init_topology_2(axiom_topology_t *start_topology) {
    /*  { 1, 2, 3, AXTP_NULL_NODE},
        { 0, 4, 8, AXTP_NULL_NODE},
        { 5, 0, 4, AXTP_NULL_NODE},
        { 0, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 2, 1, 5, AXTP_NULL_NODE},
        { 2, 4, 6, 7},
        { 8, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 5, 9, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 6, 1, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 7, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE}  */
    /* node 0 */
    start_topology->topology[0][0] = 1;
    start_topology->topology[0][1] = 2;
    start_topology->topology[0][2] = 3;
    /* node 1 */
    start_topology->topology[1][0] = 0;
    start_topology->topology[1][1] = 4;
    start_topology->topology[1][2] = 8;
    /* node 2 */
    start_topology->topology[2][0] = 5;
    start_topology->topology[2][1] = 0;
    start_topology->topology[2][2] = 4;
    /* node 3 */
    start_topology->topology[3][0] = 0;
    /* node 4 */
    start_topology->topology[4][0] = 2;
    start_topology->topology[4][1] = 1;
    start_topology->topology[4][2] = 5;
    /* node 5 */
    start_topology->topology[5][0] = 2;
    start_topology->topology[5][1] = 4;
    start_topology->topology[5][2] = 6;
    start_topology->topology[5][3] = 7;
    /* node 6 */
    start_topology->topology[6][0] = 8;
    start_topology->topology[6][1] = 5;
    /* node 7 */
    start_topology->topology[7][0] = 5;
    start_topology->topology[7][1] = 9;
    /* node 8 */
    start_topology->topology[8][0] = 6;
    start_topology->topology[8][1] = 1;
    /* node 9 */
    start_topology->topology[9][0] = 7;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_2;

    printf ("Init topology 2\n");
}

/* Initializes start_toplogy (conf 3) with the connected nodes */
void axsw_init_topology_3(axiom_topology_t *start_topology) {
    /*  { 1, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 3, 3, 4},
        { 0, 4, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 1, 4, 5},
        { 3, 1, 2, AXTP_NULL_NODE},
        { 3, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE} */
    /* node 0 */
    start_topology->topology[0][0] = 1;
    start_topology->topology[0][1] = 2;
    /* node 1 */
    start_topology->topology[1][0] = 0;
    start_topology->topology[1][1] = 3;
    start_topology->topology[1][2] = 3;
    start_topology->topology[1][3] = 4;
    /* node 2 */
    start_topology->topology[2][0] = 0;
    start_topology->topology[2][1] = 4;
    /* node 3 */
    start_topology->topology[3][0] = 1;
    start_topology->topology[3][1] = 1;
    start_topology->topology[3][2] = 4;
    start_topology->topology[3][3] = 5;
    /* node 4 */
    start_topology->topology[4][0] = 3;
    start_topology->topology[4][1] = 1;
    start_topology->topology[4][2] = 2;
    /* node 5 */
    start_topology->topology[5][0] = 3;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_3;

    printf ("Init topology 3\n");

}

/* Initializes start_toplogy (conf 4) with the connected nodes */
void axsw_init_topology_4(axiom_topology_t *start_topology) {
    /*  { 1, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 4, 3, 3},
        { 4, 0, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 4, 1, 1, 5},
        { 3, 1, 2, AXTP_NULL_NODE},
        { 3, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE} */
    /* node 0 */
    start_topology->topology[0][0] = 1;
    start_topology->topology[0][1] = 2;
    /* node 1 */
    start_topology->topology[1][0] = 0;
    start_topology->topology[1][1] = 4;
    start_topology->topology[1][2] = 3;
    start_topology->topology[1][3] = 3;
    /* node 2 */
    start_topology->topology[2][0] = 4;
    start_topology->topology[2][1] = 0;
    /* node 3 */
    start_topology->topology[3][0] = 4;
    start_topology->topology[3][1] = 1;
    start_topology->topology[3][2] = 1;
    start_topology->topology[3][3] = 5;
    /* node 4 */
    start_topology->topology[4][0] = 3;
    start_topology->topology[4][1] = 1;
    start_topology->topology[4][2] = 2;
    /* node 5 */
    start_topology->topology[5][0] = 3;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_4;

    printf ("Init topology 4\n");
}

/* Initializes start_toplogy (conf 5) with the connected nodes */
void
axsw_init_topology_5(axiom_topology_t *start_topology) {
    /*  { 1, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 0, 2, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 1, 3, 4, AXTP_NULL_NODE},
        { 2, AXTP_NULL_NODE, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 2, 5, AXTP_NULL_NODE, AXTP_NULL_NODE},
        { 4, 0, AXTP_NULL_NODE, AXTP_NULL_NODE} */
    /* node 0 */
    start_topology->topology[0][0] = 1;
    start_topology->topology[0][1] = 5;
    /* node 1 */
    start_topology->topology[1][0] = 0;
    start_topology->topology[1][1] = 2;
    /* node 2 */
    start_topology->topology[2][0] = 1;
    start_topology->topology[2][1] = 3;
    start_topology->topology[2][2] = 4;
    /* node 3 */
    start_topology->topology[3][0] = 2;
    /* node 4 */
    start_topology->topology[4][0] = 2;
    start_topology->topology[4][1] = 5;
    /* node 5 */
    start_topology->topology[5][0] = 4;
    start_topology->topology[5][1] = 0;

    start_topology->num_nodes = AXTP_NUM_NODES_SIM_5;

    printf ("Init topology 5\n");
}

/* Initialization of pointer to the topology management functions */
void
axsw_init_f_topology (axsw_sim_topology_t *sim_toplogy) {
    int i;
    for (i = 0; i < AXTP_MAX_NUM_SIM; i++) {
        sim_toplogy->axsw_f_init_topology[i] = NULL;
        sim_toplogy->needed_switch_port[i] = 0;
    }

    sim_toplogy->axsw_f_init_topology[0] = axsw_init_topology_0;
    sim_toplogy->needed_switch_port[0] = AXTP_NUM_PORT_SIM_0;
    sim_toplogy->axsw_f_init_topology[1] = axsw_init_topology_1;
    sim_toplogy->needed_switch_port[1] = AXTP_NUM_PORT_SIM_1;
    sim_toplogy->axsw_f_init_topology[2] = axsw_init_topology_2;
    sim_toplogy->needed_switch_port[2] = AXTP_NUM_PORT_SIM_2;
    sim_toplogy->axsw_f_init_topology[3] = axsw_init_topology_3;
    sim_toplogy->needed_switch_port[3] = AXTP_NUM_PORT_SIM_3;
    sim_toplogy->axsw_f_init_topology[4] = axsw_init_topology_4;
    sim_toplogy->needed_switch_port[4] = AXTP_NUM_PORT_SIM_4;
    sim_toplogy-> axsw_f_init_topology[5] = axsw_init_topology_5;
    sim_toplogy->needed_switch_port[5] = AXTP_NUM_PORT_SIM_5;
}
