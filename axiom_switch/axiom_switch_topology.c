/*!
 * \file axiom_switch_topology.c
 *
 * \version     v0.7
 * \date        2016-05-03
 *
 * This file implements the API to manage the topology in the Axiom Switch
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
#include "axiom_switch_topology.h"
#include "axiom_nic_packets.h"
#include "axiom_nic_discovery.h"

void
axsw_print_topology(axsw_logic_t *logic)
{
   int i, j;

   printf("--- AXIOM switch topology ---\n");
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
   fflush(stdout);
}

void
axsw_init_topology(axsw_logic_t *logic)
{
    int i,j;

    for (i = 0; i < AXIOM_NODES_MAX; i++) {
        for (j = 0; j < AXIOM_INTERFACES_MAX; j++) {
            logic->start_topology.topology[i][j] = AXIOM_NULL_NODE;
        }
    }
    logic->start_topology.num_nodes =  AXIOM_NODES_MAX;
    logic->start_topology.num_interfaces =  AXIOM_INTERFACES_MAX;
}

void
axsw_make_ring_topology(axsw_logic_t *logic, int num_nodes)
{
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
    logic->start_topology.num_interfaces =  AXIOM_INTERFACES_MAX;

}

int
axsw_check_mesh(int num_nodes, uint8_t* rows, uint8_t* columns)
{
    int i;
    int sq = (int)sqrt((double)num_nodes);

    for (i = sq; i >= 2; i--) {
        if (num_nodes % i == 0) {
            *rows = i;
            *columns = num_nodes / (*rows);
            return 0;
        }
    }
    return -1;
}

void
axsw_make_mesh_topology(axsw_logic_t *logic, uint8_t rows, uint8_t columns)
{
    /* The structure of the start topology is based on the following hypotesis:
     * each node interfaces are numbered in the following way
     *                      |IF1
     *                      |
     *           IF2 ---- Node X ---- IF0
     *                      |
     *                      |IF3
     */
    int i, node_index;
    int num_nodes = rows * columns;

    for (node_index = 0; node_index < num_nodes; node_index++) {
        /* **************** IF0 of each node ******************/
        /* general rule for IF0 */
        logic->start_topology.topology[node_index][0] =  node_index + 1;
        for (i = 0; i < rows; i++) {
            if (node_index == ((i*columns)+(columns-1))) {
                /* IF0 rule for nodes of the last column of the topology */
                logic->start_topology.topology[node_index][0] = (i*columns);
            }
        }

        /* **************** IF1 of each node ******************/
        if (node_index < columns) {
            /* IF1 rule for nodes of the first row of the topology */
            logic->start_topology.topology[node_index][1] =
                ((rows-1)*columns)+node_index;
        } else {
            /* general rule for IF1 */
            logic->start_topology.topology[node_index][1] = node_index - columns;
        }

        /* **************** IF2 of each node ******************/
        if (node_index != 0) {
            /* general rule for IF2 */
            logic->start_topology.topology[node_index][2] =  node_index - 1;
        }

        for (i = 0; i < rows; i++) {
            if (node_index == (i*columns)) {
                /* IF2 rule for nodes of the first column of the topology */
                logic->start_topology.topology[node_index][2] =
                    (node_index + columns -1);
            }
        }

        /* **************** IF3 of each node ******************/
        /* IF3 rule for nodes of the last row of the topology */
        if ((node_index >= ((rows-1)*columns)) && (node_index < num_nodes)) {
            logic->start_topology.topology[node_index][3] =
                node_index - ((rows-1)*columns);
        } else {
            /* general rule for IF3 */
            logic->start_topology.topology[node_index][3] =
                node_index + columns;
        }
    }

    logic->start_topology.num_nodes =  num_nodes;
    logic->start_topology.num_interfaces =  AXIOM_INTERFACES_MAX;
}


int
axsw_make_topology_from_file(axsw_logic_t *logic, char *filename) {

    FILE *file = NULL;
    char *line = NULL, *p;
    size_t len = 0;
    ssize_t read;
    int line_count = 0, if_index = 0;

    file = fopen(filename, "r");
    if (file == NULL)  {
        printf("File not existent \n");
        return -1;
    }

    while ((read = getline(&line, &len, file)) != -1) {
       line_count++;
       if (line_count > AXIOM_NODES_MAX) {
           printf("The topology contains more than %d nodes\n", AXIOM_NODES_MAX);
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
                   printf("Error in converting nodes id read from file\n");
                   return -1;
               }
               if ((val < 0) || (val > AXIOM_NODES_MAX)) {
                   printf("The topology contains nodes with id greater than %d\n",
                           AXIOM_NODES_MAX);
                   return -1;
               }
               if (if_index >= AXIOM_INTERFACES_MAX) {
                   printf("The topology contains nodes with more than  than %d \
                           interfaces\n", AXIOM_INTERFACES_MAX);
                   return -1;
               }
               logic->start_topology.topology[line_count-1][if_index] =
                   (uint8_t)val;
               if_index++;
           } else {
               /* move on to the next character of the line */
               p++;
           }
       }
    }
    logic->start_topology.num_interfaces =  AXIOM_INTERFACES_MAX;
    logic->start_topology.num_nodes = line_count;
    free(line);
    fclose(file);

    return line_count;
}
