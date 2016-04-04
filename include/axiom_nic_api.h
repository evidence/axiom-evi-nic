#ifndef AXIOM_NIC_API_h
#define AXIOM_NIC_API_h

/*
 * axiom_nic_api.h
 *
 * Last update: 2016-03-08
 *
 * This file contains the AXIOM NIC API
 *
 */

/*****************************Return Values ***********************************/
#define AXIOM_RET_ERROR       0       /* error */
#define AXIOM_RET_OK          1       /*no error  */

/********************************* Types **************************************/
typedef uint8_t		axiom_raw_type_t;	/* Raw message type */
typedef uint8_t		axiom_node_id_t;	/* Node identifier */
typedef uint8_t		axiom_msg_id_t;	/* Message identifier */
typedef uint8_t		axiom_if_id_t;	/* Interface identifier */
typedef uint8_t*	axiom_addr_t;	/* Address memory type */
typedef uint32_t	axiom_data_t;	/* DATA type */
typedef uint8_t		axiom_err_t;	/* Axiom Error type */
typedef struct axiom_dev axiom_dev_t;     /* Axiom device private data pointer */

/*
 * @brief This function sends raw data to a remote node.
 * @param dev The axiom devive private data pointer
 * @param type type of the raw message
 * @param src_node_id The local node Id that will send raw data
 * @param dst_node_id The remote node id that will receive the raw data
 * @param data data to be sent
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t src_node_id,
        axiom_node_id_t dst_node_id, axiom_raw_type_t type, axiom_data_t data);

/*
 * @brief This function receives raw data to a remote node.
 * @param dev The axiom devive private data pointer
 * @param type type of the raw message
 * @param src_node_id The source node Id that sent raw data
 * @param dst_node_id The destination node id that received the raw data
 * @param data data received
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_node_id_t *dst_node_id, axiom_raw_type_t *type, axiom_data_t *data);

/*
 * @brief This function stores data to a remote node’s memory.
 * @param dev The axiom devive private data pointer
 * @param src_node The local node Id that sends data to a remote node
 * @param dst_node The remote node’s id where data will be stored
 * @param local_src_addr The local address from where data will be transmitted
 * @param remore_dst_addr The remote address where data will be stored
 * @param payload_size Size of data to be sent in words
 * @return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t local_src_addr,
        axiom_addr_t remote_dst_addr, uint16_t payload_size);

/*
 * @brief This function requests data from a remote node to be stored locally.
 * @param dev The axiom devive private data pointer
 * @param src_node The local node Id that requests data from a remote node
 * @param dst_node The remote node id that will send the requested data
 * @param remote_src_addr The remote address from where data will be fetched
 * @param local_dst_addr The local address where fetched data will be stored
 * @param payload_size Size of data to be fetched in words
 * @return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_req(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t remore_src_addr,
        axiom_addr_t local_dst_addr, uint16_t payload_size);

/*
 * @brief This function reads the NI status register.
 * @param dev The axiom devive private data pointer
 */
uint32_t
axiom_read_ni_status(axiom_dev_t *dev);

/*
 * @brief This function reads the HW counter value associated with a specific
 *        RDMA request id.
 * @param dev The axiom devive private data pointer
 * @param msg_id The RDMA request id that is pending data.
 * @return Returns the HW counter value associated with a specific RDMA request
 *         id.
 */
uint32_t
axiom_read_hw_counter(axiom_dev_t *dev, axiom_msg_id_t msg_id);

/*
 * @brief This function sets the control registers for enabling local
 *        transmission ACKs and/or the PHY loopback mode configuration.
 * @param dev The axiom devive private data pointer
 * @param reg_mask The register mask to be used for the configuration.
 */
void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask);

/*
 * @brief This function reads the control registers
 * @param dev The axiom devive private data pointer
 */
uint32_t
axiom_read_ni_control(axiom_dev_t *dev);

/*
 * @brief This function sets the id of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id The Id assigned to this node.
 */
void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id);

/*
 * @brief This function returns the local node id.
 * @param dev The axiom devive private data pointer
 * return Returns the local node id.
 */
axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev);

/*
 * @brief This function sets the routing table of a particular node.
 * @param dev The axiom devive private data pointer
 * @param dst_node Remote node id to setup its routing table
 * @param nodeRoutingTable  Routing table to be sent
 */
//void setRoutingTable(axiom_node_id_t dst_node, uint32_t* nodeRoutingTable);
/*XXX: NOT NECESSARY */

/*
 * @brief This function sets the routing table of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id Remote connected node id
 * @param enabled_mask bit mask interface
 */
axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask);

/*
 * @brief This function returns the routing table of a node.
 * @param dev The axiom devive private data pointer
 * @param dst_node Remote node id to read its routing table
 * return Returns the routing table of a node.
 */
//uint32_t* getRoutingTable(axiomnode_id_t dst_node);
/*XXX: NOT NECESSARY */

/*
 * @brief This function gets the routing table of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id Remote connected node id
 * @param enabled_mask bit mask interface
 */
axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask);

/*
 * @brief This function updates the routing table of a node for a particular
 *        entry.
 * @param dst_node Remote node id to update its routing table
 * @param nodeIdToUpdate  Entry to update within the routing table
 */
//void updateRoutingTable(axiom_node_id_t dst_node, axiom_node_id_t nodeIdToUpdate);
/*XXX: NOT NECESSARY */

/*
 * @brief This function invalidates an entry of the dst_node routing table.
 * @param dst_node Remote node id to remove entry from its routing table
 * @param nodeIdToUpdate  Node id that no longer belongs to the routing table of
 *        dst_node
 */
//void deleteNodeFromRoutingTable(axiom_node_id_t dst_node,
//		axiom_node_id_t nodeIdToRemove);
/*XXX: NOT NECESSARY */

/*
 * @brief This function sends an identification packet to the neighbor node
 *        connected to an IF, and receives its id.
 * @param dev The axiom devive private data pointer
 * @param if_id Online interface Id where the identification message will be sent
 * return Returns the id of the neighbor node .
 */
//axiom_node_id_t identifyNeighborNode(axiom_dev_t *dev, axiom_if_id_t if_id);
/*XXX: NOT NECESSARY */

/*
 * @brief This function transmits the node’s neighbor – interface pairs to the
 *        "master node".
 * @param dev The axiom devive private data pointer
 */
//void reportNeighbors(axiom_dev_t *dev);
/*XXX: NOT NECESSARY */

/*
 * @brief This function reads the number of interfaces which are present on a
 * node.
 * @param dev The axiom devive private data pointer
 * @param if_number The number of the node interfaces
 * return Returns ...
 */
axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number);

/*
 * @brief This function reads the features of an interface.
 * @param dev The axiom devive private data pointer
 * @param if_number The number of the node interface
 * @param if_features The features of the node interface
 * return Returns ...
 */
axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features);

/*
 * @brief This function sends a raw message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
axiom_msg_id_t
axiom_send_raw_neighbour(axiom_dev_t *dev, axiom_if_id_t src_interface,
        axiom_raw_type_t type, axiom_data_t data);

/*
 * @brief This function receives a raw message from a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Pointer to the remote sender interface identification
 * @param dst_interface Pointer to the local receiver interface identification
 * @param type type of the raw message
 * @param data Received data
 * return Returns ...
 */
axiom_msg_id_t
axiom_recv_raw_neighbour (axiom_dev_t *dev, axiom_if_id_t *src_interface,
        axiom_if_id_t *dst_interface, axiom_raw_type_t *type,
        axiom_data_t *data);


#endif /* !AXIOM_NIC_API_h */