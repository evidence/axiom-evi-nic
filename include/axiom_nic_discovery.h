#ifndef AXIOM_NIC_DISCOVERY_h
#define AXIOM_NIC_DISCOVERY_h

/*!
 * \file axiom_nic_discovery.h
 *
 * \version     v0.5
 * \date        2016-03-08
 *
 * This file contains the AXIOM NIC API for the discovery phase
 *
 */
#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_small_commands.h"

/********************************* Types **************************************/
typedef uint8_t	  axiom_discovery_cmd_t;    /*!< \brief Discovery command type*/



/********************************* Packet *************************************/

/*! \brief Message payload for the discovery protocol */
typedef struct axiom_discovery_payload {
    uint8_t command;    /*!< \brief Command of discovery messages */
    uint8_t src_node;   /*!< \brief Source node id */
    uint8_t dst_node;   /*!< \brief Destination node id */
    uint8_t src_dst_if; /*!< \brief Source interface | Dest Interface */
} axiom_discovery_payload_t;



/******************************* Functions ************************************/

/*!
 * \brief This function sends a discovery message to a neighbour on a specific
 *        interface.
 *
 * \param dev             The axiom device private data pointer
 * \param interface       Sender interface where to send the message
 * \param cmd             Command of discovery message
 * \param payload_src_id  Source node id to put into the payload
 * \param payload_dst_id  Destination node id to put into the payload
 * \param payload_src_if  Soruce interface id to put into the payload
 * \param payload_dst_if  Destination interface id to put into the payload
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_send_small_discovery (axiom_dev_t *dev, axiom_if_id_t interface,
        axiom_discovery_cmd_t cmd, axiom_node_id_t payload_src_id,
        axiom_node_id_t payload_dst_id, axiom_if_id_t payload_src_if,
        axiom_if_id_t payload_dst_if)
{
    axiom_discovery_payload_t payload;
    axiom_msg_id_t ret;


    payload.command = cmd;
    payload.src_node = payload_src_id;
    payload.dst_node = payload_dst_id;
    payload.src_dst_if  = ((axiom_if_id_t)((0x0F & payload_src_if) << 4) |
                                (0x0F & payload_dst_if)) ;


    ret = axiom_send_small(dev, interface, AXIOM_SMALL_PORT_INIT,
            AXIOM_SMALL_FLAG_NEIGHBOUR, (axiom_payload_t*)(&payload));


    DPRINTF("ret: %x payload: %x", ret, (*(uint32_t*)&payload));

    return ret;
}

/*!
 * \brief This function receive a discovery message to a neighbour
 *        on a specific interface.
 *
 * \param dev             The axiom device private data pointer
 * \param interface       Receiver interface where the message is received
 * \param cmd             Command of discovery message
 * \param payload_src_id  Source node id from the payload
 * \param payload_dst_id  Destination node id from the payload
 * \param payload_src_if  Soruce interface id from the payload
 * \param payload_dst_if  Destination interface id from the payload
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_recv_small_discovery(axiom_dev_t *dev, axiom_if_id_t *interface,
        axiom_discovery_cmd_t *cmd, axiom_node_id_t *src_id,
        axiom_node_id_t *dst_id, axiom_if_id_t *payload_src_if,
        axiom_if_id_t *payload_dst_if)
{
    axiom_discovery_payload_t payload;
    axiom_port_t port;
    axiom_flag_t flag;
    axiom_msg_id_t ret;

    port = AXIOM_SMALL_PORT_INIT;
    flag = AXIOM_SMALL_FLAG_NEIGHBOUR;
    ret = axiom_recv_small(dev, interface, &port, &flag,
            (axiom_payload_t*)(&payload));

    if ((ret == AXIOM_RET_OK) && (port == AXIOM_SMALL_PORT_INIT))
    {
        /* payload info */
        *cmd = payload.command;
        *src_id    = payload.src_node;
        *dst_id    = payload.dst_node;
        *payload_src_if = (payload.src_dst_if >> 4) & 0x0F;
        *payload_dst_if = (payload.src_dst_if) & 0x0F;

        return AXIOM_RET_OK;
    }

    EPRINTF("ret: %x port: %x flag: %x payload: %x", ret, port, flag,
            (*(uint32_t*)&payload));
    return AXIOM_RET_ERROR;
}

#endif /* !AXIOM_NIC_DISCOVERY_h */
