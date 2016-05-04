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

/* Initializes start_topology with no connected nodes */
void
axsw_init_topology(axsw_logic_t *logic)
{
    int i,j;

    for (i = 0; i < AXIOM_MAX_NODES; i++) {
        for (j = 0; j < AXIOM_MAX_INTERFACES; j++) {
            logic->start_topology.topology[i][j] = AXIOM_NULL_NODE;
        }
    }
    logic->start_topology.num_nodes =  AXIOM_MAX_NODES;
    logic->start_topology.num_interfaces =  AXIOM_MAX_INTERFACES;
}

/* Initialize a ring of 'num_nodes' nodes */
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
    logic->start_topology.num_interfaces =  AXIOM_MAX_INTERFACES;

}

/* functions for checking if the inserted number of nodes */
/* is ok for a mesh topology */
int axsw_check_mesh_number_of_nodes(int number_of_nodes, uint8_t* row,
                                    uint8_t* columns)
{
    int i;
    int sq = (int)sqrt((double)number_of_nodes);

    for (i = sq; i >= 2; i--) {
        if (number_of_nodes % i == 0) {
            *row = i;
            *columns = number_of_nodes / (*row);
            return 0;
        }
    }
    return -1;
}

/* Initialize a ring of 'num_nodes' nodes */
/* The structure of the start topology is based on the
 * following hypotesis: each node interfaces are numbered
 * in the following way
 *                      |IF1
 *                      |
 *           IF2 -------|------- IF0
 *                      |
 *                      |IF3
 */
void
axsw_make_mesh_topology(axsw_logic_t *logic, int num_nodes,
                        uint8_t row, uint8_t columns)
{
    int i, node_index;

    for (node_index = 0; node_index < num_nodes; node_index++) {
        /* **************** IF0 of each node ******************/
        /* general rule for IF0 */
        logic->start_topology.topology[node_index][0] =  node_index + 1;
        for (i = 0; i < row; i++) {
            if (node_index == ((i*columns)+(columns-1))) {
                /* IF0 rule for nodes of the last column of the topology */
                logic->start_topology.topology[node_index][0] = (i*columns);
            }
        }

        /* **************** IF1 of each node ******************/
        if (node_index < columns) {
            /* IF1 rule for nodes of the first row of the topology */
            logic->start_topology.topology[node_index][1] =
                ((row-1)*columns)+node_index;
        } else {
            /* general rule for IF1 */
            logic->start_topology.topology[node_index][1] = node_index - columns;
        }

        /* **************** IF2 of each node ******************/
        if (node_index != 0) {
            /* general rule for IF2 */
            logic->start_topology.topology[node_index][2] =  node_index - 1;
        }

        for (i = 0; i < row; i++) {
            if (node_index == (i*columns)) {
                /* IF2 rule for nodes of the first column of the topology */
                logic->start_topology.topology[node_index][2] =
                    (node_index + columns -1);
            }
        }

        /* **************** IF3 of each node ******************/
        /* IF3 rule for nodes of the last row of the topology */
        if ((node_index >= ((row-1)*columns)) && (node_index < num_nodes)) {
            logic->start_topology.topology[node_index][3] =
                node_index - ((row-1)*columns);
        } else {
            /* general rule for IF3 */
            logic->start_topology.topology[node_index][3] =
                node_index + columns;
        }
    }

    logic->start_topology.num_nodes =  num_nodes;
    logic->start_topology.num_interfaces =  AXIOM_MAX_INTERFACES;
}


/* Initializes start_topology with the file input topology
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
        printf("File not existent \n");
        return -1;
    }

    while ((read = getline(&line, &len, file)) != -1) {
       line_count++;
       if (line_count > AXIOM_MAX_NODES) {
           printf("The topology contains more than %d nodes\n", AXIOM_MAX_NODES);
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
               if ((val < 0) || (val > AXIOM_MAX_NODES)) {
                   printf("The topology contains nodes with id greater than %d\n",
                           AXIOM_MAX_NODES);
                   return -1;
               }
               if (if_index >= AXIOM_MAX_INTERFACES) {
                   printf("The topology contains nodes with more than  than %d \
                           interfaces\n", AXIOM_MAX_INTERFACES);
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
    logic->start_topology.num_interfaces =  AXIOM_MAX_INTERFACES;
    logic->start_topology.num_nodes = line_count;
    free(line);
    fclose(file);

    return line_count;
}
