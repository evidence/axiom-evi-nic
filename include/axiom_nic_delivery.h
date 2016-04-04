#ifndef AXIOM_NIC_DELIVERY_H
#define AXIOM_NIC_DELIVERY_H

/*
 * axiom_nic_delivery.h
 *
 * Last update: 2016-03-10
 *
 * This file contains the AXIOM NIC API for the delivery
 *
 */

/********************************* Types **************************************/
typedef uint8_t		axiom_raw_delivery_t;	/* Raw message dilivery-type */

/*
 * @brief This function sends a discovery message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_raw_delivery (axiom_dev_t *dev, axiom_raw_delivery_t data_type,
                     axiom_node_id_t src_node_id, axiom_node_id_t dst_node_id,
                      axiom_node_id_t data_node_id, axiom_if_id_t data_if_id)
{
    axiom_msg_id_t ret;
    axiom_rt_delivery_msg_t msg;

    msg.data.r_table.type = data_type;
    msg.data.r_table.node_id = data_node_id;
    msg.data.r_table.if_id = data_if_id;

    ret = axiom_send_raw (dev, src_node_id, dst_node_id,
                          AXIOM_RAW_TYPE_ROUTING, msg.data.raw);

    return ret;
}

/*
 * @brief This function is used for routing table reception;
          it receives raw packets from a remote node.
 * @param dev The axiom device private data pointer
 * @param data_type sub-type of the raw message
 * @param src_node_id The remote node Id that has sent raw data
 * @param dst_node_id The local node id that will receive the raw data
 * @param node_id id of the node present into the 'dst_node_id' node
          routing table
 * @param if_id id of the 'dst_node_id' node interface which is connected
          with the 'node_id' node
* @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
inline static axiom_msg_id_t
axiom_recv_raw_delivery(axiom_dev_t *dev, axiom_raw_delivery_t *data_type,
                         axiom_node_id_t *src_node_id, axiom_node_id_t *dst_node_id,
                         axiom_node_id_t *data_node_id, axiom_if_id_t *data_if_id)
{
    axiom_rt_delivery_msg_t msg;
    axiom_msg_id_t ret;

    /* receive routing info with raw messgees */
    ret = axiom_recv_raw (dev, src_node_id,
                          dst_node_id,
                          &msg.header.raw.type,
                          &msg.data.raw);

    *data_type = msg.data.r_table.type;
    *data_node_id = msg.data.r_table.node_id;
    *data_if_id = msg.data.r_table.if_id;

    return ret;
}

#endif /* !AXIOM_NIC_DELIVERY_H */
