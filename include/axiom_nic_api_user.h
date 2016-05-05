#ifndef AXIOM_NIC_API_USER_h
#define AXIOM_NIC_API_USER_h
/*!
 * \file axiom_nic_api_user.h
 *
 * \version     v0.5
 * \date        2016-03-14
 *
 * This file contains the AXIOM NIC API for the userspace
 *
 */

#include "axiom_nic_types.h"


/*!
 * \brief  axiom arguments for the axiom_open() function
 */
/*TODO: is it needed ? */
struct axiom_args {

};

/*!
 * \brief This function open an axiom device
 *
 * \param args          The axiom open arguments
 *
 * \return Returns a pointer to the axiom device private data pointer,
 *         NULL otherwise.
 */
axiom_dev_t *
axiom_open(axiom_args_t *args);

/*!
 * \brief This function close an axiom device
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_close(axiom_dev_t *dev);

/*!
 * \brief This function bind the current process on a specified port
 *
 * \param dev           The axiom device private data pointer
 * \param port          Port number
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_bind(axiom_dev_t *dev, axiom_port_t port);

/*!
 * \brief  This function return the interface to reach a specified node
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        Node id of target node
 * \param if_number     Interface id to reache a node
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_next_hop(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_if_id_t *if_number);

/*!
 * \brief  This function sends small data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the small data or
 *                      local interface that will send the small data
 * \param port          port of the small message
 * \param flag          flags of the small message
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_send_small(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_t port, axiom_flag_t flag, axiom_payload_t *payload);

/*!
 * \brief This function receives small data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the small data or local
 *                      interface that received the small data
 * \param port          port of the small message
 * \param flag          flags of the small message
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_recv_small(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_flag_t *flag, axiom_payload_t *payload);

/*!
 * \brief This function writes data to a remote node memory.
 *
 * \param dev              The axiom device private data pointer
 * \param src_node         The local node Id that sends data to a remote node
 * \param dst_node         The remote node’s id where data will be stored
 * \param local_src_addr   The local address from where data will be transmitted
 * \param remore_dst_addr  The remote address where data will be stored
 * \param payload_size     Size of data to be sent in words
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t local_src_addr,
        axiom_addr_t remote_dst_addr, uint16_t payload_size);

/*!
 * \brief This function requests data from a remote node to be stored locally.
 *
 * \param dev             The axiom device private data pointer
 * \param src_node        The local node Id that requests data from a remote node
 * \param dst_node        The remote node id that will send the requested data
 * \param remote_src_addr The remote address from where data will be fetched
 * \param local_dst_addr  The local address where fetched data will be stored
 * \param payload_size    Size of data to be fetched in words
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_req(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t remore_src_addr,
        axiom_addr_t local_dst_addr, uint16_t payload_size);

/*!
 * \brief This function reads the NI status register.
 *
 * \param               dev The axiom device private data pointer
 *
 * \return Returns the status register value.
 */
uint32_t
axiom_read_ni_status(axiom_dev_t *dev);

/*!
 * \brief This function reads the HW counter value associated with a specific
 *        RDMA request id.
 *
 * \param dev           The axiom device private data pointer
 * \param msg_id        The RDMA request id that is pending data.
 *
 * \return Returns the HW counter value associated with a specific RDMA request
 *         id.
 */
uint32_t
axiom_read_hw_counter(axiom_dev_t *dev, axiom_msg_id_t msg_id);

/*!
 * \brief This function sets the control registers for enabling local
 *        transmission ACKs and/or the PHY loopback mode configuration.
 *
 * \param dev           The axiom device private data pointer
 * \param reg_mask The register mask to be used for the configuration.
 */
void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask);

/*!
 * \brief This function reads the control register.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the control register value.
 */
uint32_t
axiom_read_ni_control(axiom_dev_t *dev);

/*!
 * \brief This function sets the id of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       The Id assigned to this node.
 */
void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id);

/*!
 * \brief This function returns the local node id.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the local node id.
 */
axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev);

#if 0
/*!
 * \brief This function sets the routing table of a particular node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_node      Remote node id to setup its routing table
 * \param nodeRoutingTable  Routing table to be sent
 */
void setRoutingTable(axiom_node_id_t dst_node, uint32_t* nodeRoutingTable);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function sets the routing table of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       Remote connected node id
 * \param enabled_mask  bit mask interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask);

#if 0
/*!
 * \brief This function returns the routing table of a node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_node      Remote node id to read its routing table
 *
 * \return Returns the routing table of a node.
 */
uint32_t* getRoutingTable(axiomnode_id_t dst_node);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function gets the routing table of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       Remote connected node id
 * \param enabled_mask  bit mask interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask);

#if 0
/*!
 * \brief This function updates the routing table of a node for a particular
 *        entry.
 *
 * \param dst_node       Remote node id to update its routing table
 * \param nodeIdToUpdate Entry to update within the routing table
 */
void updateRoutingTable(axiom_node_id_t dst_node, axiom_node_id_t nodeIdToUpdate);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function invalidates an entry of the dst_node routing table.
 *
 * \param dst_node       Remote node id to remove entry from its routing table
 * \param nodeIdToUpdate Node id that no longer belongs to the routing table of
 *        dst_node
 */
void deleteNodeFromRoutingTable(axiom_node_id_t dst_node,
                                axiom_node_id_t nodeIdToRemove);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function sends an identification packet to the neighbor node
 *        connected to an IF, and receives its id.
 *
 * \param dev           The axiom device private data pointer
 * \param if_id         Online interface Id where the identification message will be sent
 *
 * \return Returns the id of the neighbor node .
 */
axiom_node_id_t identifyNeighborNode(axiom_dev_t *dev, axiom_if_id_t if_id);
/*XXX: NOT NECESSARY */

/*!
 * \brief This function transmits the node’s neighbor – interface pairs to the
 *        "master node".
 *
 * \param dev           The axiom device private data pointer
 */
void reportNeighbors(axiom_dev_t *dev);
/*XXX: NOT NECESSARY */
#endif

/*!
 * \brief This function reads the number of interfaces which are present on a
 *        node.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The number of the node interfaces
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number);

/*!
 * \brief This function reads the features of an interface.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The number of the node interface
 * \param if_features   The features of the node interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features);


#endif /* !AXIOM_NIC_API_USER_h */
