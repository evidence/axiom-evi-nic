/*!
 * \file axiom_sim_topology.h
 *
 * \version     v0.13
 * \date        2016-03-25
 *
 * This file contains the AXIOM NIC topology structure and functions. It is
 * used only for the QEMU emulation, inside the Axiom switch.
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_SIM_TOPOLOGY_h
#define AXIOM_SIM_TOPOLOGY_H

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/*!
 * \brief Axiom topology structure
 *
 * This structure is used to encode the network topology in the axiom-switch
 */
typedef struct axiom_topology {
    /*!
     * \brief network topology: nodes info
     *
     * Contains the node topology encoded in this way:
     * topology[NX][IY] = NZ  -> means node NX is connected on the interface IY
     *                           with node NZ
     */
    uint8_t topology[AXIOM_NODES_MAX][AXIOM_INTERFACES_MAX];
    /*! \brief network topology: interfaces info
     *
     * Contains the node topology encoded in this way:
     * if_topology[NX][IY] = IZ  -> means node NX is connected on the interface
     *                              IY with interface IZ on the remote node
     *                              written in topology[NX][IY]
     */
    uint8_t if_topology[AXIOM_NODES_MAX][AXIOM_INTERFACES_MAX];
    int num_nodes;      /*!< \brief number of nodes in the network */
    int num_interfaces; /*!< \brief number of node interfaces */
} axiom_topology_t;


/*!
 * \brief This function initializes the if_topology field in the
 *        axiom_topology_t structure.
 *
 * \param topology      Axiom topology structure
 * \param num_nodes     Number of nodes in the network
 */
inline static void
axsw_if_topology_init(axiom_topology_t *topology, int num_nodes)
{
    int node_index, if_index, i;
    uint8_t pair_node;

    for (node_index = 0; node_index < num_nodes; node_index++) {
        for (if_index = 0; if_index < AXIOM_INTERFACES_MAX; if_index++) {
            topology->if_topology[node_index][if_index] = AXIOM_NULL_NODE;
        }
    }

    for (node_index = 0; node_index < num_nodes; node_index++) {

        for (if_index = 0; if_index < AXIOM_INTERFACES_MAX; if_index++) {

            pair_node = topology->topology[node_index][if_index];
            if (pair_node == AXIOM_NULL_NODE)
                continue;

            for (i = 0; i < AXIOM_INTERFACES_MAX; i++) {

                if ((topology->topology[pair_node][i] == node_index) &&
                    (topology->if_topology[pair_node][i] == AXIOM_NULL_NODE)) {
                    /* interface of the connected nodes */
                    topology->if_topology[node_index][if_index] = i;
                    topology->if_topology[pair_node][i] = if_index;
                    break;
                }
            }
        }
    }
}

/** \} */

#endif  /* !AXIOM_SIM_TOPOLOGY_h */
