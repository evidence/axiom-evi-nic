
/*
 * Default implementation of axiom switch logic
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

#include <netinet/in.h>

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_nic_packets.h"
#include "axiom_nic_discovery.h"

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
    dst_vm = logic->start_topology.topology[src_vm][dst_if];

    NDPRINTF("src_vm_index: %d dst_if: %d dst_vm: %d", src_vm, dst_if, dst_vm);

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

static void
print_computed_toplogy(axsw_logic_t *logic)  {
   int i, j;

   printf("*** Start Toplogy ***\n");
   printf("Node");
   for (j = 0; j < logic->start_topology.num_interfaces; j++) {
       printf("\tIF%d", j);
   }
   printf("\n");

   for (i = 0; i < logic->start_topology.num_nodes; i++) {
       printf("%d", i);
       for (j = 0; j < logic->start_topology.num_interfaces; j++) {
           printf("\t%u", logic->start_topology.topology[i][j]);
       }
       printf("\n");
   }
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
axsw_init_topology(axsw_logic_t *logic) {
    int i,j;

    for (i = 0; i < AXIOM_MAX_NODES; i++) {
        for (j = 0; j < AXIOM_MAX_INTERFACES; j++) {
            logic->start_topology.topology[i][j] = AXTP_NULL_NODE;
        }
    }
    logic->start_topology.num_nodes =  AXIOM_MAX_NODES;
    logic->start_topology.num_interfaces =  AXTP_NUM_INTERFACES;
}

/* Initialize a ring of 'num_nodes' nodes */
void
axsw_make_ring_toplogy(axsw_logic_t *logic, int num_nodes) {

    int i;

    /* first direction */
    for (i = 0; i < num_nodes-1; i++) {
        logic->start_topology.topology[i][0] =  i+1;
    }
    logic->start_topology.topology[num_nodes-1][0] = 0;

    /* second direction */
    logic->start_topology.topology[0][1] = num_nodes-1;
    for (i = 1; i < num_nodes; i++) {
        logic->start_topology.topology[i][1] =  i-1;
    }

    logic->start_topology.num_nodes =  num_nodes;
    logic->start_topology.num_interfaces =  AXTP_NUM_INTERFACES;

    print_computed_toplogy(logic);
}

/* Initializes start_toplogy with the file input topology
   and returns the number of nodes into */
int
axsw_topology_from_file(axsw_logic_t *logic, char *filename) {

    FILE *file = NULL;
    char *line = NULL, *p;
    size_t len = 0;
    ssize_t read;
    int line_count = 0, if_index = 0;

    file = fopen(filename, "r");
    if (file == NULL)  {
        printf ("File not existent \n");
        return -1;
    }

    while ((read = getline(&line, &len, file)) != -1) {
       /* printf("%s", line); */

       line_count++;
       if (line_count > AXIOM_MAX_NODES) {
           printf ("The topology contains more than %d nodes\n", AXIOM_MAX_NODES);
           return -1;
       }
       if_index = 0;
       p = line;
       /* While there are characters into the line */
       while (*p) {
           if (isdigit(*p)) {
               /* Upon finding a digit, read it */
               long val = strtol(p, &p, 10);
               if ((val ==  LONG_MIN) || (val ==  LONG_MAX)) {
                   printf ("Error in converting nodes id read from file\n");
                   return -1;
               }
               else
               {
                   if ((val < 0) || (val > AXIOM_MAX_NODES)) {
                       printf ("The topology contains nodes with id greater than %d\n", AXIOM_MAX_NODES);
                       return -1;
                   }
                   else {
                       if (if_index >= AXTP_NUM_INTERFACES) {
                           printf ("The topology contains nodes with more than  than %d interfaces\n", AXTP_NUM_INTERFACES);
                           return -1;
                       }
                       else {
                           logic->start_topology.topology[line_count-1][if_index] = (uint8_t)val;
                           if_index++;
                       }
                   }
               }
           }
           else {
               /* move on to the next character of the line */
               p++;
           }
       }
    }
    logic->start_topology.num_interfaces =  AXTP_NUM_INTERFACES;
    logic->start_topology.num_nodes = line_count;
    free(line);
    fclose(file);

    print_computed_toplogy(logic);

    return line_count;
}
