#ifndef AXIOM_SWITCH_TOPOLOGY_h
#define AXIOM_SWITCH_TOPOLOGY_h

//#define AXTP_EXAMPLE1
//#define AXTP_EXAMPLE2
//#define AXTP_EXAMPLE3
//#define AXTP_EXAMPLE4
#define AXTP_EXAMPLE5

#ifdef AXTP_EXAMPLE1
#define AXTP_NUM_NODES               8
#endif
#ifdef AXTP_EXAMPLE2
#define AXTP_NUM_NODES               10
#endif
#ifdef AXTP_EXAMPLE3
#define AXTP_NUM_NODES               6
#endif
#ifdef AXTP_EXAMPLE4
#define AXTP_NUM_NODES               6
#endif
#ifdef AXTP_EXAMPLE5
#define AXTP_NUM_NODES               6
#endif

#define AXTP_NUM_INTERFACES          4   /* Max number of connected node interfaces */
#define AXTP_NULL_NODE               255 /* Virtual machine (node) not connected */

typedef struct axiom_topology {
    uint8_t topology[AXTP_NUM_NODES][AXTP_NUM_INTERFACES];
    int num_nodes;
    int num_interfaces;
} axiom_topology_t;


#endif  /* !AXIOM_SWITCH_TOPOLOGY_H */
