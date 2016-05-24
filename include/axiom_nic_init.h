#ifndef AXIOM_NIC_INIT_h
#define AXIOM_NIC_INIT_h

/*!
 * \file axiom_nic_init.h
 *
 * \version     v0.5
 * \date        2016-04-13
 *
 * This file contains the AXIOM NIC types for axiom-init deamon
 *
 */
#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_small_commands.h"


/********************************* Types **************************************/
typedef uint8_t		axiom_init_cmd_t;	/*!< \brief init command type*/



/********************************* Packets *************************************/

/*! \brief Generic message payload for the axiom-init deamon */
typedef struct axiom_init_payload {
    uint8_t  command;           /*!< \brief Command of messages */
    uint8_t  spare[3];
} axiom_init_payload_t;


/*! \brief Message payload for the axiom-ping application */
typedef struct axiom_ping_payload {
    uint8_t  command;           /*!< \brief Command of ping-pong messages */
    uint8_t  spare;             /*!< \brief Spare field */
    uint16_t packet_id;         /*!< \brief Packet id */
} axiom_ping_payload_t;


/*! \brief Message payload for the axiom-traceroute application */
typedef struct axiom_traceroute_payload {
    uint8_t  command;           /*!< \brief Command of traceroute messages */
    uint8_t  src_id;            /*!< \brief Source node id */
    uint8_t  dst_id;            /*!< \brief Destination node id */
    uint8_t  step;              /*!< \brief Step of the message route */
} axiom_traceroute_payload_t;


/*! \brief Message payload for the axiom-netperf application */
typedef struct axiom_netperf_payload {
    uint8_t  command;           /*!< \brief Command of netperf messages */
    uint8_t  offset;            /*!< \brief Packet offset */
    uint16_t data;              /*!< \brief Packet data */
} axiom_netperf_payload_t;


/******************************* Functions ************************************/

/*!
 * \brief This function sends an init message.
 *
 * \param dev           The axiom device private data pointer
 * \param dst           Local interface or remote id identification
 * \param type          Type of the small message
 * \param payload       Payload to send
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_send_small_init(axiom_dev_t *dev, axiom_node_id_t dst, axiom_type_t type,
        axiom_init_payload_t *payload)
{
    axiom_msg_id_t ret;

    ret = axiom_send_small(dev, dst, AXIOM_SMALL_PORT_INIT, type,
            sizeof(*payload), payload);

    DPRINTF("ret: %x payload: %x", ret, (*(uint32_t*)&payload));

    return ret;
}

/*!
 * \brief This function receives an init message
 *
 * \param dev           The axiom device private data pointer
 * \param src           Local interface or remote id identification
 * \param type          Type of the small message
 * \param cmd           Command of the small message
 * \param payload       Payload received
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_recv_small_init(axiom_dev_t *dev, axiom_node_id_t *src, axiom_type_t *type,
        axiom_init_cmd_t *cmd, axiom_init_payload_t *payload)
{
    axiom_port_t port = AXIOM_SMALL_PORT_INIT;
    axiom_msg_id_t ret;
    axiom_payload_size_t payload_size = sizeof(*payload);

    ret = axiom_recv_small(dev, src, &port, type, &payload_size, payload);

    if ((ret != AXIOM_RET_OK) || (port != AXIOM_SMALL_PORT_INIT))
    {
        EPRINTF("ret: %x port: %x type: %x", ret, port, *type);
        return AXIOM_RET_ERROR;
    }

    /* payload info */
    *cmd = payload->command;

    return AXIOM_RET_OK;
}
#endif /* !AXIOM_NIC_INIT_h */
