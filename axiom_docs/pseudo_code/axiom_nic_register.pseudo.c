#include "axiom_nic_api_hw.h"
#include "axiom_nic_regs.h"

/*!
 * axiom_nic_register.pseudo.c
 *
 * \version     v0.7
 * \date        2016-03-08
 *
 * This file contains the pseudo code of AXIOM NIC API HW
 *
 */

/******************************* FORTH API ************************************/

/*
 * @brief This function sends raw data to a remote node.
 * @param dev The axiom devive private data pointer
 * @param src_node The local node Id that will send raw data
 * @param dst_node The remote node id that will receive the raw data
 * @param data data to be sent
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_data_t data)
{
    uint8_t queue_status, head;
    axiom_raw_hdr_t header;

    /* check space available in the queue */
    queue_status = axiom_io_read8(dev, AXIOMNET_IO_RAW_TX_INFO);
    if (queue_status & AXIOMNET_RAW_STATUS_FULL) {
        return -1;
    }

    /* read the queue head */
    head = axiom_io_read8(dev, AXIOMNET_IO_RAW_TX_HEAD);

    /* fill the header */
    header.src_node = src_node;
    header.dst_node = dst_node;
    header.type = AXIOM_RAW_TYPE_GENERIC;
    header.spare = 0;

    /* write the header in the descriptor[head] */
    axiom_io_write32(dev, AXIOMNET_IO_RAW_TX_BASE + head, (uint32_t)header);
    /* write the data in the descriptor[head] */
    axiom_io_write32(dev, AXIOMNET_IO_RAW_TX_BASE + head + 1, data);

    /* send the notification to the NIC */
    axiom_io_write8(dev, AXIOMNET_IO_RAW_TX_START, 1);

    return head;
}

/*
 * @brief This function receives raw data to a remote node.
 * @param dev The axiom devive private data pointer
 * @param src_node The source node Id that sent raw data
 * @param dst_node The destination node id that received the raw data
 * @param data data received
 * @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
axiom_msg_id_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_node,
        axiom_node_id_t *dst_node, axiom_data_t *data)
{
    uint8_t queue_status, tail;
    axiom_raw_hdr_t header;

    /* check packets available in the queue */
    queue_status = axiom_io_read8(dev, AXIOMNET_IO_RAW_RX_INFO);
    if (queue_status & AXIOMNET_RAW_STATUS_EMPTY) {
        return -1;
        /* TODO: block the thread? */
    }

    /* read the queue tail */
    tail = axiom_io_read8(dev, AXIOMNET_IO_RAW_RX_TAIL);

    /* read the header in the descriptor[tail] */
    header = axiom_io_read32(dev, AXIOMNET_IO_RAW_RX_BASE + tail);
    /* read the data in the descriptor[tail] */
    *data = axiom_io_read32(dev, AXIOMNET_IO_RAW_RX_BASE + tail + 1);

    /* send the notification to the NIC */
    axiom_io_write8(dev, AXIOMNET_IO_RAW_RX_START, 1);

    *src_node = header.src_node;
    *dst_node = header.dst_node;

    return tail;
}

/*
 * @brief This function stores data to a remote node’s memory.
 * @param dev The axiom devive private data pointer
 * @param src_node The local node Id that sends data to a remote node
 * @param dst_node The remote node’s id where data will be stored
 * @param local_src_addr The local address from where data will be transmitted
 * @param remote_dst_addr The remote address where transmitted data will be stored
 * @param payloadSize Size of data to be sent in words
 * @return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t local_src_addr,
        axiom_addr_t remote_dst_addr, uint16_t payloadSize);

/*
 * @brief This function requests data from a remote node to be stored locally.
 * @param dev The axiom devive private data pointer
 * @param src_node The local node Id that requests data from a remote node
 * @param dst_node The remote node id that will send the requested data
 * @param remote_src_addr The remote address from where data will be fetched
 * @param local_dst_addr The local address where fetched data will be stored
 * @param payloadSize Size of data to be fetched in words
 * @return Returns a unique positive message id on success, -1 otherwise.
 */
axiom_msg_id_t
axiom_rdma_req(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_addr_t remote_src_addr,
        axiom_addr_t local_dst_addr, uint16_t payloadSize);

/*
 * @brief This function reads the NI status register.
 * @param dev The axiom devive private data pointer
 */
uint32_t
axiom_read_ni_status(axiom_dev_t *dev)
{
    return axiom_io_read32(dev, AXIOMNET_IO_STATUS);
}

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
axiom_set_ni_register(axiom_dev_t *dev, uint8_t reg_mask)
{
    axiom_io_write32(dev, AXIOMNET_IO_CONTROL, reg_mask);
}

/*
 * @brief This function sets the id of a local node.
 * @param dev The axiom devive private data pointer
 * @param node_id The Id assigned to this node.
 */
void
set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    axiom_io_write8(dev, AXIOMNET_IO_NODEID, node_id);
}

/*
 * @brief This function returns the local node id.
 * @param dev The axiom devive private data pointer
 * return Returns the local node id.
 */
axiom_node_id_t
get_node_id(axiom_dev_t *dev)
{
    return axiom_io_read8(dev, AXIOMNET_IO_NODEID);
}

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
{
    axiom_io_write8(dev, AXIOMNET_IO_ROUTING + node_id, enabled_mask);
    return 0;
}

/*
 * @brief This function returns the routing table of a node.
 * @param dev The axiom devive private data pointer
 * @param dst_node Remote node id to read its routing table
 * return Returns the routing table of a node.
 */
//uint32_t* getRoutingTable(axiom_node_id_t dst_node);
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
{
    *enabled_mask = axiom_io_read8(dev, AXIOMNET_IO_ROUTING + node_id);
    return 0;
}

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
 * @param nodeIdToUpdate  _node id that no longer belongs to the routing table of
 *        dst_node
 */
//void delete_nodeFromRoutingTable(axiom_node_id_t dst_node,
//		axiom_node_id_t nodeIdToRemove);
/*XXX: NOT NECESSARY */

/*
 * @brief This function sends an identification packet to the neighbor node
 *        connected to an IF, and receives its id.
 * @param dev The axiom devive private data pointer
 * @param ifId Online interface Id where the identification message will be sent
 * return Returns the id of the neighbor node .
 */
//axiom_node_id_t identifyNeighbor_node(axiom_dev_t *dev, axiomIfId_t ifId);
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
get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number);
{
    *if_number = axiom_io_read8(dev, AXIOMNET_IO_IFNUMBER);
    return 0;
}

/*
 * @brief This function reads the features of an interface.
 * @param dev The axiom devive private data pointer
 * @param if_number The number of the node interface
 * @param if_features The features of the node interface
 * return Returns ...
 */
axiom_err_t
get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number, uint8_t *if_features);
{
    *if_features = axiom_io_read8(dev, AXIOMNET_IO_IFINFO + if_number);
    return 0;
}

/*
 * @brief This function sends a raw message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param type Type of discovery message
 * @param src_node_id The local node Id that requests data from a remote node
 * @param dst_node_id The remote node id that will send the requested data
 * @param src_interface Sender interface identification
 * @param dst_interface Receiver interface identification
 * @param data Data to send (used with message type AXIOM_DSCV_TYPE_SETID
 *             or AXIOM_DSCV_TYPE_END_TOPOLOGY)
 * return Returns ...
 */
axiom_msg_id_t
axiom_send_raw_neighbour(axiom_dev_t *dev, uint8_t type,
        axiom_node_id_t src_node_id, axiom_node_id_t dst_node_id,
        axiom_if_id_t src_interface, axiom_if_id_t dst_interface, uint8_t data);
{
    uint8_t queue_status, head;
    axiom_discovery_msg_t packet;

    /* check space available in the queue */
    queue_status = axiom_io_read8(dev, AXIOMNET_IO_RAW_TX_INFO);
    if (queue_status & AXIOMNET_RAW_STATUS_FULL) {
        return -1;
    }

    /* read the queue head */
    head = axiom_io_read8(dev, AXIOMNET_IO_RAW_TX_HEAD);

    /* fill the RAW header */
    packet.header.src_node = src_node;
    packet.header.dst_node = dst_node;
    packet.header.type = AXIOM_RAW_TYPE_DISCOVERY;
    packet.header.spare = 0;

    /* fill the discovery packet */
    packet.type = type;
    packet.src_if = src_interface;
    packet.dst_if = dst_interface;
    packet.data = data;

    /* write the header in the descriptor[head] */
    axiom_io_write32(dev, AXIOMNET_IO_RAW_TX_BASE + head,
            (uint32_t)packet.header);
    /* write the data in the descriptor[head] */
    axiom_io_write32(dev, AXIOMNET_IO_RAW_TX_BASE + head + 1,
            *((uint32_t *)(&packet) + 1);

    /* send the notification to the NIC */
    axiom_io_write8(dev, AXIOMNET_IO_RAW_TX_START, 1);

    return head;
}

/*
 * @brief This function receives a raw message from a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param type type of discovery message
 * @param src_node_id Pointer to the remote node Id that sends data from a remote node
 * @param dst_node_id Pointer to the local node id that will receive the sent data
 * @param src_interface Pointer to the remote sender interface identification
 * @param dst_interface Pointer to the local receiver interface identification
 * @param data Received data (used with message type AXIOM_DSCV_TYPE_SETID
 *             or AXIOM_DSCV_TYPE_END_TOPOLOGY)
 * return Returns ...
 */
axiom_msg_id_t
axiom_recv_raw_neighbour (axiom_dev_t *dev, uint8_t* type,
        axiom_node_id_t* src_node_id, axiom_node_id_t* dst_node_id,
        axiom_if_id_t* src_interface, axiom_if_id_t* dst_interface,
        uint8_t* data);
{
    uint8_t queue_status, tail;
    axiom_discovery_msg_t packet;

    /* check packets available in the queue */
    queue_status = axiom_io_read8(dev, AXIOMNET_IO_RAW_RX_INFO);
    if (queue_status & AXIOMNET_RAW_STATUS_EMPTY) {
        return -1;
        /* TODO: block the thread? */
    }

    /* read the queue tail */
    tail = axiom_io_read8(dev, AXIOMNET_IO_RAW_RX_TAIL);

    /* read the header in the descriptor[tail] */
    packet.header = axiom_io_read32(dev, AXIOMNET_IO_RAW_RX_BASE + tail);
    /* read the data in the descriptor[tail] */
    *((uint32_t *)(&packet) + 1) = axiom_io_read32(dev, AXIOMNET_IO_RAW_RX_BASE + tail + 1);

    /* send the notification to the NIC */
    axiom_io_write8(dev, AXIOMNET_IO_RAW_RX_START, 1);

    *src_node_id = packet.header.src_node;
    *dst_node_id = packet.header.dst_node;
    *type = packet.type;
    *src_interface = packet.scr_if;
    *dst_interface = packet.dst_if;
    *data = packet.data;

    return tail;
}

