#ifndef AXIOM_SWITCH_TOPOLOGY_h
#define AXIOM_SWITCH_TOPOLOGY_h
/*!
 * \file axiom_switch_topology.h
 *
 * \version     v0.4
 * \date        2016-05-03
 *
 * This file contains API to manage the topology in the Axiom Switch
 */

/* functions for topology management */
void axsw_init_topology(axsw_logic_t *logic);
void axsw_print_topology(axsw_logic_t *logic);
void axsw_make_ring_topology(axsw_logic_t *logic, int num_nodes);
int axsw_check_mesh_number_of_nodes(int number_of_nodes, uint8_t* row,
        uint8_t* columns);
void axsw_make_mesh_topology(axsw_logic_t *logic, int num_nodes, uint8_t row,
        uint8_t columns);
int axsw_topology_from_file(axsw_logic_t *logic, char *filename);

#endif /* AXIOM_SWITCH_TOPOLOGY_h */
