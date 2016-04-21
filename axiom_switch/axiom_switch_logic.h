#ifndef AXIOM_SWITCH_LOGIC_h
#define AXIOM_SWITCH_LOGIC_h

/* actual number of defined topology */
#define AXTP_NUM_SIM        6
/* maximum topologies number */
#define AXTP_MAX_NUM_SIM    10
/* number of nodes for each simulation */
#define AXTP_NUM_NODES_SIM_0    4
#define AXTP_NUM_NODES_SIM_1    8
#define AXTP_NUM_NODES_SIM_2    10
#define AXTP_NUM_NODES_SIM_3    6
#define AXTP_NUM_NODES_SIM_4    6
#define AXTP_NUM_NODES_SIM_5    6

/* switch port needed for each toploogy simualation case */
#define AXTP_NUM_PORT_SIM_0     AXTP_NUM_NODES_SIM_0
#define AXTP_NUM_PORT_SIM_1     AXTP_NUM_NODES_SIM_1
#define AXTP_NUM_PORT_SIM_2     AXTP_NUM_NODES_SIM_2
#define AXTP_NUM_PORT_SIM_3     AXTP_NUM_NODES_SIM_3
#define AXTP_NUM_PORT_SIM_4     AXTP_NUM_NODES_SIM_4
#define AXTP_NUM_PORT_SIM_5     AXTP_NUM_NODES_SIM_5

/* possible toplogies types */
#define AXTP_DEFAULT_SIM        0
#define AXTP_RING_SIM           1

typedef struct axsw_logic {
    int    vm_sd[AXSW_PORT_MAX];
    int    node_sd[AXSW_PORT_MAX];
    axiom_topology_t start_topology;
} axsw_logic_t;

typedef struct axsw_sim_topology {
    /* array of pointer to topology initialization functions */
    void (*axsw_f_init_topology[AXTP_MAX_NUM_SIM])(axsw_logic_t *logic);
    int needed_switch_port[AXTP_MAX_NUM_SIM];
} axsw_sim_topology_t;

inline static void
axsw_logic_init(axsw_logic_t *logic, int num_ports) {
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        logic->vm_sd[i] = -1;
        logic->node_sd[i] = -1;
    }
    axsw_if_topology_init(&logic->start_topology, num_ports);
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

inline static int
axsw_logic_find_neighbour_if(axsw_logic_t *logic, int src_vm, int source_if)
{
    int ret_if_index;

    ret_if_index = logic->start_topology.if_topology[src_vm][source_if];
    if (ret_if_index != AXTP_NULL_NODE) {
        return ret_if_index;
    }

    return -1;
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

/* find the node id, given its associated socket descriptor */
inline static int
axsw_logic_find_node_id(axsw_logic_t *logic, int sd)
{
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        if (logic->node_sd[i] == sd) {
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

inline static void
axsw_logic_clean_vm_sd(axsw_logic_t *logic, int sd)
{
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        if (logic->vm_sd[i] == sd) {
            logic->vm_sd[i] = -1;
        }
        if (logic->node_sd[i] == sd) {
            logic->node_sd[i] = -1;
        }
    }

}

/* functions for topology management */
void axsw_init_topology(axsw_logic_t *logic);
void axsw_print_topology(axsw_logic_t *logic);
void axsw_make_ring_topology(axsw_logic_t *logic, int num_nodes);
int axsw_check_mesh_number_of_nodes(int number_of_nodes, uint8_t* row,
        uint8_t* columns);
void axsw_make_mesh_topology(axsw_logic_t *logic, int num_nodes, uint8_t row,
        uint8_t columns);
int axsw_topology_from_file(axsw_logic_t *logic, char *filename);

#endif /* AXIOM_SWITCH_LOGIC_h */
