/*
 * axiom_topology.h
 *
 * Version:     v0.3
 * Last update: 2016-03-25
 *
 * This file contains the AXIOM NIC topology information
 *
 */
#ifndef AXIOM_TOPOLOGY_h
#define AXIOM_TOPOLOGY_H

//#define EXAMPLE0
//#define EXAMPLE1
//#define EXAMPLE2
//#define EXAMPLE3
//#define EXAMPLE4
//#define EXAMPLE5

#ifdef EXAMPLE0
#define AXIOM_NUM_NODES               4
#endif
#ifdef EXAMPLE1
#define AXIOM_NUM_NODES               8
#endif
#ifdef EXAMPLE2
#define AXIOM_NUM_NODES               10
#endif
#ifdef EXAMPLE3
#define AXIOM_NUM_NODES               6
#endif
#ifdef EXAMPLE4
#define AXIOM_NUM_NODES               6
#endif
#ifdef EXAMPLE5
#define AXIOM_NUM_NODES               6
#endif
#define AXIOM_NUM_INTERFACES          4


/* Invalid node ID */
#define AXIOM_NULL_NODE               255

typedef struct axiom_topology {
    uint8_t topology[AXIOM_MAX_NUM_NODES][AXIOM_NUM_INTERFACES];
    uint8_t if_topology[AXIOM_MAX_NUM_NODES][AXIOM_NUM_INTERFACES];
    int num_nodes;
    int num_interfaces;
} axiom_topology_t;

//extern axiom_topology_t start_topology;

inline static void
axsw_if_topology_init(axiom_topology_t *topology, int num_nodes)
{
    int node_index, if_index, i;
    uint8_t pair_node;

    for (node_index = 0; node_index < num_nodes; node_index++)
    {
        for (if_index = 0; if_index < AXIOM_NUM_INTERFACES; if_index++)
        {
            topology->if_topology[node_index][if_index] = AXIOM_NULL_NODE;
        }
    }

    for (node_index = 0; node_index < num_nodes; node_index++)
    {
        for (if_index = 0; if_index < AXIOM_NUM_INTERFACES; if_index++)
        {
            pair_node = topology->topology[node_index][if_index];
            if (pair_node != AXIOM_NULL_NODE)
            {
                for (i = 0; i < AXIOM_NUM_INTERFACES; i++)
                {
                    if ((topology->topology[pair_node][i] == node_index)
                            && (topology->if_topology[pair_node][i] == AXIOM_NULL_NODE))
                    {
                        /* interface of the connected nodes */
                        topology->if_topology[node_index][if_index] = i;
                        topology->if_topology[pair_node][i] = if_index;
                        break;
                    }
                }
            }
        }
    }
}

#endif  /* !AXIOM_TOPOLOGY_h */

