/*!
 * \file axiom_nic_discovery.h
 *
 * \version     v0.11
 * \date        2016-03-08
 *
 * This file contains the AXIOM NIC API for the discovery phase
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_DISCOVERY_h
#define AXIOM_NIC_DISCOVERY_h

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_raw_commands.h"

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
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
inline static axiom_err_t
axiom_send_raw_discovery (axiom_dev_t *dev, axiom_if_id_t interface,
        axiom_discovery_cmd_t cmd, axiom_node_id_t payload_src_id,
        axiom_node_id_t payload_dst_id, axiom_if_id_t payload_src_if,
        axiom_if_id_t payload_dst_if)
{
    axiom_discovery_payload_t payload;
    axiom_err_t ret;


    payload.command = cmd;
    payload.src_node = payload_src_id;
    payload.dst_node = payload_dst_id;
    payload.src_dst_if  = ((axiom_if_id_t)((0x0F & payload_src_if) << 4) |
                                (0x0F & payload_dst_if)) ;


    ret = axiom_send_raw(dev, interface, AXIOM_RAW_PORT_INIT,
            AXIOM_TYPE_RAW_NEIGHBOUR, sizeof(payload), &payload);


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
 * \return Returns AXIOM_RET_OK on success, an error otherwise.
 */
inline static axiom_err_t
axiom_recv_raw_discovery(axiom_dev_t *dev, axiom_if_id_t *interface,
        axiom_discovery_cmd_t *cmd, axiom_node_id_t *src_id,
        axiom_node_id_t *dst_id, axiom_if_id_t *payload_src_if,
        axiom_if_id_t *payload_dst_if)
{
    axiom_discovery_payload_t payload;
    axiom_port_t port;
    axiom_type_t type;
    axiom_err_t ret;
    axiom_raw_payload_size_t payload_size = sizeof(payload);

    port = AXIOM_RAW_PORT_INIT;
    type = AXIOM_TYPE_RAW_NEIGHBOUR;
    ret = axiom_recv_raw(dev, interface, &port, &type, &payload_size,
            &payload);

    if ((ret < AXIOM_RET_OK) || (port != AXIOM_RAW_PORT_INIT) ||
            (payload_size != sizeof(payload))) {
        EPRINTF("ret: %x port: %x[%x] type: %x payload_size: %d[%d] payload: %x",
                ret, port, AXIOM_RAW_PORT_INIT, type, payload_size,
                ((int)sizeof(payload)), (*(uint32_t*)&payload));
        return AXIOM_RET_ERROR;
    }

    /* payload info */
    *cmd = payload.command;
    *src_id    = payload.src_node;
    *dst_id    = payload.dst_node;
    *payload_src_if = (payload.src_dst_if >> 4) & 0x0F;
    *payload_dst_if = (payload.src_dst_if) & 0x0F;

    return AXIOM_RET_OK;
}

/** \} */

#endif /* !AXIOM_NIC_DISCOVERY_h */
