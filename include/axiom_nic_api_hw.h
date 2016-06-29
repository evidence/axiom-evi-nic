#ifndef AXIOM_NIC_API_HW_h
#define AXIOM_NIC_API_HW_h

/*!
 * \file axiom_nic_api_hw.h
 *
 * \version     v0.6
 * \date        2016-03-14
 *
 * This file contains the AXIOM NIC HARDWARE API
 *
 */
#include "axiom_nic_types.h"


/*!
 * \brief  This function sends raw data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param dst_id        The remote node id that will receive the raw data or
 *                      local interface that will send the raw data
 * \param port_type     port and type of the raw message
 * \param payload_size  size of data to be sent
 * \param payload       data to be sent
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_hw_send_raw(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_type_t port_type, axiom_raw_payload_size_t payload_size,
        axiom_payload_t *payload);

/*!
 * \brief This function checks the space available in the raw TX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there is space available, 0 otherwise.
 */
axiom_raw_len_t
axiom_hw_raw_tx_avail(axiom_dev_t *dev);

/*!
 * \brief This function receives raw data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param src_id        The source node id that sent the raw data or local
 *                      interface that received the raw data
 * \param port_type     port and type of the raw message
 * \param payload_size  size of data received
 * \param payload       data received
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_hw_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_type_t *port_type, axiom_raw_payload_size_t *payload_size,
        axiom_payload_t *payload);

/*!
 * \brief This function cheks the messages available in the raw RX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there are messages available, 0 otherwise.
 */
axiom_raw_len_t
axiom_hw_raw_rx_avail(axiom_dev_t *dev);

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
axiom_hw_rdma_write(axiom_dev_t *dev, axiom_node_id_t src_node,
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
axiom_hw_rdma_req(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t remore_src_addr,
        axiom_addr_t local_dst_addr, uint16_t payload_size);

/*!
 * \brief This function reads the NI status register.
 *
 * \param dev           The axiom device private data pointer
 */
uint32_t
axiom_hw_read_ni_status(axiom_dev_t *dev);

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
axiom_hw_read_hw_counter(axiom_dev_t *dev, axiom_msg_id_t msg_id);

/*!
 * \brief This function sets the control registers for enabling local
 *        transmission ACKs and/or the PHY loopback mode configuration.
 *
 * \param dev           The axiom device private data pointer
 * \param reg_mask      The register mask to be used for the configuration.
 */
void
axiom_hw_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask);

/*!
 * \brief This function reads the control registers
 *
 * \param dev           The axiom device private data pointer
 * \return Returns the control register value.
 */
uint32_t
axiom_hw_read_ni_control(axiom_dev_t *dev);

/*!
 * \brief This function sets the zone where the HW can access in RDMA.
 *
 * \param dev           The axiom device private data pointer
 * \param start         Start physical address of RDMA zone
 * \param end           End physical address of RDMA zone
 */
void
axiom_hw_set_rdma_zone(axiom_dev_t *dev, uint64_t start, uint64_t end);

/*!
 * \brief This function sets the id of a local node.
 *
 * \param dev           The axiom device private data pointer
 * \param node_id       The Id assigned to this node.
 */
void
axiom_hw_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id);

/*!
 * \brief This function returns the local node id.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns the local node id.
 */
axiom_node_id_t
axiom_hw_get_node_id(axiom_dev_t *dev);

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
axiom_hw_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask);

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
axiom_hw_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask);

/*!
 * \brief This function reads the number of interfaces which are present on a
 * node.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The number of the node interfaces
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_hw_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number);

/*!
 * \brief This function reads the features of an interface.
 *
 * \param dev           The axiom device private data pointer
 * \param if_number     The node interface identifier
 * \param if_features   The features of the node interface
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_hw_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features);


#endif /* !AXIOM_NIC_API_HW_h */
