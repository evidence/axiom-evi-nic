#ifndef AXIOM_NIC_ROUTING_H
#define AXIOM_NIC_ROUTING_H

/*
 * axiom_nic_routing.h
 *
 * Version:     v0.3.1
 * Last update: 2016-03-18
 *
 * This file contains the AXIOM NIC API for the delivery
 *
 */

#include "axiom_nic_types.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_small_commands.h"

/********************************* Types **************************************/
typedef uint8_t		axiom_routing_cmd_t;	/* Routing command */


/******************************** Packets *************************************/
typedef struct axiom_routing_payload {
    uint8_t command;            /* Command of routing messages */
    uint8_t node_id;            /* Node ID to set into the routing table */
    uint8_t if_id;              /* ID of the interface of node_id node*/
    uint8_t spare;
} axiom_routing_payload_t;


/*
 * @brief This function send a delivery message
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the small message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_small_delivery(axiom_dev_t *dev, axiom_node_id_t dst_node_id,
        axiom_routing_cmd_t cmd, axiom_node_id_t payload_node_id,
        axiom_if_id_t payload_if_id)
{
    axiom_routing_payload_t payload;
    axiom_msg_id_t ret;

    payload.command = cmd;
    payload.node_id = payload_node_id;
    payload.if_id = payload_if_id;

    ret = axiom_send_small(dev, dst_node_id, AXIOM_SMALL_PORT_INIT,
            0,  (axiom_payload_t*)(&payload));

    return ret;
}

/*
 * @brief This function is used for routing table reception;
          it receives small packets from a remote node.
 * @param dev The axiom device private data pointer
 * @param payload_type sub-type of the small message
 * @param src_node_id The remote node Id that has sent small data
 * @param dst_node_id The local node id that will receive the small data
 * @param node_id id of the node present into the 'dst_node_id' node
          routing table
 * @param if_id id of the 'dst_node_id' node interface which is connected
          with the 'node_id' node
* @return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
 */
inline static axiom_msg_id_t
axiom_recv_small_delivery(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_routing_cmd_t *cmd, axiom_node_id_t *payload_node_id,
        axiom_if_id_t *payload_if_id)
{
    axiom_routing_payload_t payload;
    axiom_port_t port;
    axiom_flag_t flag;
    axiom_msg_id_t ret;

    port = AXIOM_SMALL_PORT_INIT;
    flag = 0;
    /* receive routing info with small messgees */
    ret = axiom_recv_small(dev, src_node_id, &port, &flag, (axiom_payload_t*)(&payload));

    if ((ret == AXIOM_RET_OK) && (port == AXIOM_SMALL_PORT_INIT))
    {
        /* payload info */
        *cmd = payload.command;
        *payload_node_id = payload.node_id;
        *payload_if_id = payload.if_id;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}

/*
 * @brief This function sends a set-routing message to a neighbour
 *        on a specific interface.
 * @param dev The axiom device private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the small message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_small_set_routing(axiom_dev_t *dev, axiom_if_id_t my_interface,
        axiom_routing_cmd_t cmd)
{
    axiom_routing_payload_t payload;
    axiom_msg_id_t ret;

    payload.command = cmd;
    payload.node_id = 0;
    payload.if_id = 0;

    ret = axiom_send_small(dev, my_interface, AXIOM_SMALL_PORT_INIT,
            AXIOM_SMALL_FLAG_NEIGHBOUR, (axiom_payload_t*)(&payload));

    return ret;
}

/*
 * @brief This function sends a set-routing message to a neighbour
 *        on a specific interface.
 * @param dev The axiom device private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the small message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_recv_small_set_routing(axiom_dev_t *dev, axiom_if_id_t *my_interface,
        axiom_routing_cmd_t *cmd)
{
    axiom_routing_payload_t payload;
    axiom_port_t port;
    axiom_flag_t flag;
    axiom_msg_id_t ret;

    port = AXIOM_SMALL_PORT_INIT;
    flag = AXIOM_SMALL_FLAG_NEIGHBOUR;
    /* receive routing info with small messgees */
    ret = axiom_recv_small(dev, my_interface, &port, &flag, (axiom_payload_t*)(&payload));

    if ((ret == AXIOM_RET_OK) && (port == AXIOM_SMALL_PORT_INIT))
    {
        /* payload info */
        *cmd = payload.command;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}
#endif /* !AXIOM_NIC_ROUTING_H */
