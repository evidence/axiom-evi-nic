/*!
 * \file axiom_nic_api_hw.h
 *
 * \version     v0.13
 * \date        2016-03-14
 *
 * This file contains the AXIOM NIC HARDWARE API
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_API_HW_h
#define AXIOM_NIC_API_HW_h

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

#include "axiom_nic_types.h"


/*!
 * \brief  This function sends raw data to a remote node.
 *
 * \param dev           The axiom device private data pointer
 * \param msg           Message to send (header + payload)
 *
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_raw_tx(axiom_dev_t *dev, axiom_raw_msg_t *msg);

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
 * \param msg           Message received (header + payload)
 *
 * \return Returns a unique positive message id on success.
 */
axiom_msg_id_t
axiom_hw_raw_rx(axiom_dev_t *dev, axiom_raw_msg_t *msg);

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
 * \param dev           The axiom device private data pointer
 * \param header        The header of packet
 *
 * \return Returns a unique positive message id.
 */
axiom_msg_id_t
axiom_hw_rdma_tx(axiom_dev_t *dev, axiom_rdma_hdr_t *header);

/*!
 * \brief This function checks the space available in the RDMA TX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there is space available, 0 otherwise.
 */
axiom_queue_len_t
axiom_hw_rdma_tx_avail(axiom_dev_t *dev);

/*!
 * \brief This function reads data from a remote node memory.
 *
 * \param dev           The axiom device private data pointer
 * \param header        The header of packet
 *
 * \return Returns a unique positive message id.
 */
axiom_msg_id_t
axiom_hw_rdma_rx(axiom_dev_t *dev, axiom_rdma_hdr_t *header);

/*!
 * \brief This function checks the space available in the RDMA RX queue.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns a value != 0 if there is space available, 0 otherwise.
 */
axiom_queue_len_t
axiom_hw_rdma_rx_avail(axiom_dev_t *dev);

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
 * \brief This function sets the buffers to receive LONG messages.
 *
 * \param dev           The axiom device private data pointer
 * \param buf_id        Id of the buffer to set
 * \param long_buf      Buffer info to set (address, size, used)
 */
void
axiom_hw_set_long_buf(axiom_dev_t *dev, int buf_id,
        axiomreg_long_buf_t *long_buf);

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
 * \param enabled_mask  bit mask interface (actual implementation support only 1
 *                      node)
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
 * \param enabled_mask  bit mask interface (actual implementation support only 1
 *                      node)
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

/*!
 * \brief This function enables all interrup of the AXIOM NIC.
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_hw_enable_irq(axiom_dev_t *dev);

/*!
 * \brief This function disables all interrup of the AXIOM NIC.
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_hw_disable_irq(axiom_dev_t *dev);

/*!
 * \brief This function returns the pending interrupts.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns pending interrupts
 */
uint32_t
axiom_hw_pending_irq(axiom_dev_t *dev);

/*!
 * \brief This function acks the specified interrupts.
 *
 * \param dev           The axiom device private data pointer
 * \param ack_irq       Interrupt to acknowledge
 */
void
axiom_hw_ack_irq(axiom_dev_t *dev, uint32_t ack_irq);

/*!
 * \brief This function checks the version of the NIC.
 *
 * \param dev           The axiom device private data pointer
 *
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
axiom_err_t
axiom_hw_check_version(axiom_dev_t *dev);

/** \} */

#endif /* !AXIOM_NIC_API_HW_h */
