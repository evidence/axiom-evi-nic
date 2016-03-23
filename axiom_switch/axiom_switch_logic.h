#ifndef AXIOM_SWITCH_LOGIC_h
#define AXIOM_SWITCH_LOGIC_h

#define AXSW_PORT_MAX           16      /* max port supported */
#define AXSW_PORT_START         33300   /* first port to listen */

typedef struct axsw_logic {
    int    vm_sd[AXSW_PORT_MAX];
    int    node_sd[AXSW_PORT_MAX];
} axsw_logic_t;


inline static void
axsw_logic_init(axsw_logic_t *logic) {
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        logic->vm_sd[i] = -1;
        logic->node_sd[i] = -1;
    }
}
/* This function return the destination socket to forward the message */
int axsw_logic_forward(axsw_logic_t *logic, int src_sd,
        axiom_small_eth_t *axiom_packet);


/* find the socket descriptor, given its associated node id */
inline static int
axsw_logic_find_small_sd(axsw_logic_t *logic, int dst_node)
{
    if (dst_node >= AXSW_PORT_MAX)
        return -1;

    return logic->node_sd[dst_node];
}

/* find the socket descriptor, given its associated virtual machine */
inline static int
axsw_logic_find_neighbour_sd(axsw_logic_t *logic, int dst_vm)
{
    if (dst_vm >= AXSW_PORT_MAX)
        return -1;

    return logic->vm_sd[dst_vm];
}

/* find the dst_vm interface wich is connected to src_vm*/
inline static int
axsw_logic_find_neighbour_if(axiom_topology_t *start_topology,
                             int src_vm, int dst_vm)
{
    int i, if_index = -1;

    for (i = 0; i < AXTP_NUM_INTERFACES; i++)
    {
        if (src_vm == start_topology->topology[dst_vm][i])
        {
            if_index = i;
            break;
        }
    }

    return if_index;
}

/* find the index of virtual machine, given its associated socket descriptor */
inline static int
axsw_logic_find_vm_index(axsw_logic_t *logic, int sd)
{
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        if (logic->vm_sd[i] == sd) {
            return i;
        }
    }

    return -1;
}

/* store socket associated with the virtual machine vm_index */
inline static void
axsw_logic_set_vm_sd(axsw_logic_t *logic, int vm_index, int sd)
{
    if (vm_index < AXSW_PORT_MAX) {
        logic->vm_sd[vm_index] = sd;
    }

}

#endif /* AXIOM_SWITCH_LOGIC_h */
