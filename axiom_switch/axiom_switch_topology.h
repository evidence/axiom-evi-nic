#ifndef AXIOM_SWITCH_TOPOLOGY_h
#define AXIOM_SWITCH_TOPOLOGY_h
/*!
 * \file axiom_switch_topology.h
 *
 * \version     v0.8
 * \date        2016-05-03
 *
 * This file contains API to manage the topology in the Axiom Switch
 */

/*!
 * \brief Initialize the topology structure in the logic status
 *
 * \param logic         logic status pointer
 */
void axsw_init_topology(axsw_logic_t *logic);

/*!
 * \brief Print the topology structure in the logic status
 *
 * \param logic         logic status pointer
 */
void axsw_print_topology(axsw_logic_t *logic);

/*!
 * \brief Create a ring topology
 *
 * \param logic         logic status pointer
 * \param num_nodes     number of nodes in the network
 */
void axsw_make_ring_topology(axsw_logic_t *logic, int num_nodes);

/*!
 * \brief Check if it is possible to generate 2D mesh torus and return the
 * rows and columns of the mesh.
 *
 * \param num_nodes     number of nodes in the network
 * \param[out] rows     number of 2D mesh rows
 * \param[out] columns  number of 2D mesh columns
 *
 * \return 0 on success, otherwise -1
 */
int axsw_check_mesh(int num_nodes, uint8_t* rows, uint8_t* columns);

/*!
 * \brief Create a 2D mesh torus topology
 *
 * \param logic         logic status pointer
 * \param rows          number of 2D mesh rows
 * \param columns       number of 2D mesh columns
 */
void axsw_make_mesh_topology(axsw_logic_t *logic, uint8_t rows, uint8_t columns);

/*!
 * \brief Create a topology, parsing a file
 *
 * \param logic         logic status pointer
 * \param filename      name of file that contains the topology
 *
 * \return number of nodes on success, otherwise -1
 */
int axsw_make_topology_from_file(axsw_logic_t *logic, char *filename);

#endif /* AXIOM_SWITCH_TOPOLOGY_h */
