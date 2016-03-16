#ifndef AXIOM_SWITCH_CONFIGURATION_H
#define AXIOM_SWITCH_CONFIGURATION_H

#include <stdint.h>
#include <netinet/in.h>

//#define EXAMPLE1
//#define EXAMPLE2
//#define EXAMPLE3
//#define EXAMPLE4
#define EXAMPLE5

#ifdef EXAMPLE1
#define NUM_NODES               8
#endif
#ifdef EXAMPLE2
#define NUM_NODES               10
#endif
#ifdef EXAMPLE3
#define NUM_NODES               6
#endif
#ifdef EXAMPLE4
#define NUM_NODES               6
#endif
#ifdef EXAMPLE5
#define NUM_NODES               6
#endif

#define NUM_INTERFACES          4       /* Max number of connected
                                            node interfaces */
#define NULL_NODE               255     /* Virtual machine (node)
                                            not connected */

typedef struct topology {
    uint8_t topology[NUM_NODES][NUM_INTERFACES];
    int num_nodes;
    int num_interfaces;
} topology_t;



/* find the socket descriptor, given its associated
   node id */
inline static void
find_raw_sd(int dst_node, int *node_sd, int *dest_sd)
{
    *dest_sd = node_sd[dst_node];
    return;
}

/* find the socket descriptor, given its associated
   virtual machine */
/* find the socket descriptor, given its associated
   virtual machine */
inline static void
find_neighbour_sd(int dest_vm_index, int *vm_sd, int *dest_sd)
{
    *dest_sd = vm_sd[dest_vm_index];
    return;
}

/* find the index of virtual machine, given its associated
   socket descriptor */
inline static int
find_vm_index(int sd, int *vm_sd, uint8_t *vm_index)
{
    int i;
    int ret = -1;

    for (i = 0; i < AXSW_PORT_MAX; i++)
    {
        if (vm_sd[i] == sd)
        {
            *vm_index = i;
            ret = 0;
            break;
        }
    }

    return ret;
}

/* compute the virtual machine index associated with the socket sd */
inline static int
compute_vm_index_from_listen_sd(int sd, uint8_t *vm_index)
{
    struct sockaddr_in sin;
    int addrlen = sizeof(sin);
    int local_port;

    if ((getsockname(sd, (struct sockaddr *)&sin, &addrlen) == 0) &&
                    (sin.sin_family == AF_INET) &&
                    (addrlen == sizeof(sin)))
    {
         local_port = ntohs(sin.sin_port);
         *vm_index = local_port - AXSW_PORT_START;

         return 0;
    }

    return -1;
}

#endif  //! AXIOM_SWITCH_CONFIGURATION_H
