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
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_raw_tx(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_type_t port_type, axiom_raw_payload_size_t payload_size,
        axiom_payload_t *payload);

/*!
 * \brief This function checks the space available in the raw TX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there is space available, 0 otherwise.
 */
axiom_queue_len_t
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
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_raw_rx(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_type_t *port_type, axiom_raw_payload_size_t *payload_size,
        axiom_payload_t *payload);

/*!
 * \brief This function cheks the messages available in the raw RX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there are messages available, 0 otherwise.
 */
axiom_queue_len_t
axiom_hw_raw_rx_avail(axiom_dev_t *dev);

/*!
 * \brief This function writes data to a remote node memory.
 *
 * \param dev             The axiom device private data pointer
 * \param remote_id          The remote node id where data will be stored
 * \param port_type       port and type of the rdma message
 * \param payload_size    size of data to be transfer in 64bits words
 * \param local_src_addr  local offset inside the RDMA zone where data
 *                        will be read
 * \param remote_dst_addr remote offset inside the RDMA zone where data
 *                        will be stored
 *
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_rdma_tx(axiom_dev_t *dev, axiom_node_id_t remote_id,
        axiom_port_type_t port_type, axiom_rdma_payload_size_t payload_size,
        axiom_addr_t src_addr, axiom_addr_t dst_addr);

/*!
 * \brief This function reads data from a remote node memory.
 *
 * \param dev             The axiom device private data pointer
 * \param remote_id          The remote node id where data will be read
 * \param port_type       port and type of the rdma message
 * \param payload_size    size of data to be transfer in 64bits words
 * \param remote_src_addr remote offset inside the RDMA zone where data
 *                        will be read
 * \param local_dst_addr  local offset inside the RDMA zone where data
 *                        will be stored
 *
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_rdma_rx(axiom_dev_t *dev, axiom_node_id_t *remote_id,
        axiom_port_type_t *port_type, axiom_rdma_payload_size_t *payload_size,
        axiom_addr_t *src_addr, axiom_addr_t *dst_addr);

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
