#ifndef AXIOM_NIC_ROUTING_H
#define AXIOM_NIC_ROUTING_H

/*!
 * \file axiom_nic_routing.h
 *
 * \version     v0.5
 * \date        2016-03-18
 *
 * This file contains the AXIOM NIC API for the routing phase
 *
 */

#include "axiom_nic_types.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_small_commands.h"

/********************************* Types **************************************/
typedef uint8_t	    axiom_routing_cmd_t;     /*!< \brief Routing command type */


/******************************** Packets *************************************/

/*! \brief Message payload for the routing phase */
typedef struct axiom_routing_payload {
    uint8_t command;    /*!< \brief Command of routing messages */
    uint8_t node_id;    /*!< \brief Node ID to set into the routing table */
    uint8_t if_mask;    /*!< \brief Interface mask to reach node_id */
    uint8_t spare;      /*!< \brief Spare field */
} axiom_routing_payload_t;


/******************************* Functions ************************************/

/*!
 * \brief This function sends a message to deliver a routing table
 *
 * \param dev              The axiom device private data pointer
 * \param dst_node_id      Destination node of the routing table
 * \param cmd              Command of routing message
 * \param payload_node_id  Node to set in the routing table
 * \param payload_if_mask  Interface mask to set for the node_id
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_send_small_delivery(axiom_dev_t *dev, axiom_node_id_t dst_node_id,
        axiom_routing_cmd_t cmd, axiom_node_id_t payload_node_id,
        axiom_if_id_t payload_if_mask)
{
    axiom_routing_payload_t payload;
    axiom_msg_id_t ret;

    payload.command = cmd;
    payload.node_id = payload_node_id;
    payload.if_mask = payload_if_mask;

    ret = axiom_send_small(dev, dst_node_id, AXIOM_SMALL_PORT_INIT,
            0,  (axiom_payload_t*)(&payload));

    return ret;
}

/*!
 * \brief This function is used to receive a routing table
 *
 * \param dev              The axiom device private data pointer
 * \param src_node_id      Sender node ID
 * \param cmd              Command of routing message
 * \param payload_node_id  Node to set in the routing table
 * \param payload_if_mask  Interface mask to set for the node_id
 *
 * \return Returns a unique positive message id on success, -1 otherwise.
 * XXX: the return type is unsigned!
*/
inline static axiom_msg_id_t
axiom_recv_small_delivery(axiom_dev_t *dev, axiom_node_id_t *src_node_id,
        axiom_routing_cmd_t *cmd, axiom_node_id_t *payload_node_id,
        axiom_if_id_t *payload_if_mask)
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
        *payload_if_mask = payload.if_mask;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}

/*!
 * \brief This function sends a set-routing message to a neighbour
 *        on a specific interface
 *
 * \param dev           The axiom device private data pointer
 * \param interface     Sender interface where to send the message
 * \param cmd           Command of routing message
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_send_small_set_routing(axiom_dev_t *dev, axiom_if_id_t interface,
        axiom_routing_cmd_t cmd)
{
    axiom_routing_payload_t payload;
    axiom_msg_id_t ret;

    payload.command = cmd;
    payload.node_id = 0;
    payload.if_mask = 0;

    ret = axiom_send_small(dev, interface, AXIOM_SMALL_PORT_INIT,
            AXIOM_SMALL_FLAG_NEIGHBOUR, (axiom_payload_t*)(&payload));

    return ret;
}

/*!
 * \brief This function receives a set-routing message
 *
 * \param dev           The axiom device private data pointer
 * \param interface     Receiver interface where the message is received
 * \param cmd           Command of routing message
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_recv_small_set_routing(axiom_dev_t *dev, axiom_if_id_t *interface,
        axiom_routing_cmd_t *cmd)
{
    axiom_routing_payload_t payload;
    axiom_port_t port;
    axiom_flag_t flag;
    axiom_msg_id_t ret;

    port = AXIOM_SMALL_PORT_INIT;
    flag = AXIOM_SMALL_FLAG_NEIGHBOUR;
    /* receive routing info with small messgees */
    ret = axiom_recv_small(dev, interface, &port, &flag, (axiom_payload_t*)(&payload));

    if ((ret == AXIOM_RET_OK) && (port == AXIOM_SMALL_PORT_INIT))
    {
        /* payload info */
        *cmd = payload.command;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}
#endif /* !AXIOM_NIC_ROUTING_H */
