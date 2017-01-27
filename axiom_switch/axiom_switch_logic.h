/*!
 * \file axiom_switch_logic.h
 *
 * \version     v0.10
 * \date        2016-05-03
 *
 * This file contains API to implements the logic in the Axiom Switch
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_SWITCH_LOGIC_h
#define AXIOM_SWITCH_LOGIC_h

/*! \brief logic status structure */
typedef struct axsw_logic {
    /*! \brief Node socket descriptors array where the index is the VM id */
    int    vm_sd[AXSW_PORT_MAX];
    /*! \brief Node socket descriptors array where the index is the Node id */
    int    node_sd[AXSW_PORT_MAX];
    /*! \brief topology of the network */
    axiom_topology_t start_topology;
} axsw_logic_t;

/************************* Custom logic function ******************************/

/*!
 * \brief Get socket descriptor where to forward a received packet
 *
 * \param logic         logic status pointer
 * \param src_sd        source socket descriptor
 * \param axiom_packet  received packet to forward
 *
 * \return socket descriptor to forward the packet
 */
int axsw_logic_forward(axsw_logic_t *logic, int src_sd,
        axiom_eth_pkt_t *axiom_packet);



/************************ Generic logic functions *****************************/

/*!
 * \brief Initialize logic status structure
 *
 * \param logic         logic status pointer
 * \param num_ports     number of ports
 */
inline static void
axsw_logic_init(axsw_logic_t *logic, int num_ports) {
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        logic->vm_sd[i] = -1;
        logic->node_sd[i] = -1;
    }
    axsw_if_topology_init(&logic->start_topology, num_ports);
}

/*!
 * \brief Get the socket descriptor, given its associated node id
 *
 * \param logic         logic status pointer
 * \param dst_node      node id to find
 *
 * \return socket descriptor associated to dst_node
 */
inline static int
axsw_logic_find_node_sd(axsw_logic_t *logic, int dst_node)
{
    if (unlikely(dst_node >= AXSW_PORT_MAX))
        return -1;

    return logic->node_sd[dst_node];
}

/*!
 * \brief Get the socket descriptor, given its associated VM id
 *
 * \param logic         logic status pointer
 * \param dst_vm        VM id to find
 *
 * \return socket descriptor associated to dst_vm
 */
inline static int
axsw_logic_find_neighbour_sd(axsw_logic_t *logic, int dst_vm)
{
    if (unlikely(dst_vm >= AXSW_PORT_MAX))
        return -1;

    return logic->vm_sd[dst_vm];
}

/*!
 * \brief Get interface ID of the neighbour VM connected with the source VM on
 *        the source interface
 *
 * \param logic         logic status pointer
 * \param src_vm        source VM identifier
 * \param src_if        source interface identifier
 *
 * \return interface ID of the neighbour VM
 */
inline static int
axsw_logic_find_neighbour_if(axsw_logic_t *logic, int src_vm, int src_if)
{
    int ret_if_id;

    ret_if_id = logic->start_topology.if_topology[src_vm][src_if];
    if (unlikely(ret_if_id == AXIOM_NULL_NODE)) {
        return -1;
    }

    return ret_if_id;
}

/*!
 * \brief Get the id of virtual machine, given its associated socket descriptor
 *
 * \param logic         logic status pointer
 * \param sd            socket descriptor
 *
 * \return VM id associated to a socket descriptor
 */
inline static int
axsw_logic_find_vm_id(axsw_logic_t *logic, int sd)
{
    int i;

    for (i = 0; i < AXSW_PORT_MAX; i++) {
        if (logic->vm_sd[i] == sd) {
            return i;
        }
    }

    return -1;
}

/*!
 * \brief Get the node id, given its associated socket descriptor
 *
 * \param logic        logic status pointer
 * \param sd            socket descriptor
 *
 * \return node id associated to a socket descriptor
 */
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

/*!
 * \brief Set socket descriptor associated with the VM id
 *
 * \param logic         logic status pointer
 * \param vm_id         VM identifier
 * \param sd            socket descriptor
 */
inline static void
axsw_logic_set_vm_sd(axsw_logic_t *logic, int vm_id, int sd)
{
    if (likely(vm_id < AXSW_PORT_MAX)) {
        logic->vm_sd[vm_id] = sd;
    }

}

/*!
 * \brief Clean socket descpriptor and node/VM pairs
 *
 * \param logic         logic status pointer
 * \param sd            socket descriptor
 */
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

#endif /* AXIOM_SWITCH_LOGIC_h */
