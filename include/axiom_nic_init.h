#ifndef AXIOM_NIC_INIT_h
#define AXIOM_NIC_INIT_h

/*!
 * \file axiom_nic_init.h
 *
 * \version     v0.6
 * \date        2016-04-13
 *
 * This file contains the AXIOM NIC types for axiom-init deamon
 *
 */
#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_raw_commands.h"


/********************************* Types **************************************/
typedef uint8_t		axiom_init_cmd_t;   /*!< \brief init command type*/

typedef enum {
    AXNP_RDMA = 1,
    AXNP_LONG,
    AXNP_RAW
} axiom_netperf_type_t;                     /*!< \brief axiom netperf type */


/********************************* Packets *************************************/

/*! \brief Generic message payload for the axiom-init deamon */
typedef struct axiom_init_payload {
    uint8_t  command;           /*!< \brief Command of messages */
    uint8_t  spare[127];
} axiom_init_payload_t;


/*! \brief Message payload for the axiom-ping application */
typedef struct axiom_ping_payload {
    uint8_t  command;           /*!< \brief Command of ping-pong messages */
    uint8_t  padding[3];
    uint32_t unique_id;         /*!< \brief Unique id */
    uint32_t seq_num;           /*!< \brief Sequence number */
    uint64_t timestamp;         /*!< \brief Timestamp */
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
    uint8_t  padding[7];
    uint64_t total_bytes;       /*!< \brief Total bytes of the stream */
    uint64_t elapsed_time;      /*!< \brief Time elapsed to receive data */
    uint8_t  type;              /*!< \brief Type of message used in the test */
    uint8_t  magic;             /*!< \brief Magic byte write in the payload */
    uint8_t  error;             /*!< \brief Error report */
    uint8_t  spare[101];
} axiom_netperf_payload_t;


/******************************* Functions ************************************/

/*!
 * \brief This function sends an init message.
 *
 * \param dev           The axiom device private data pointer
 * \param dst           Local interface or remote id identification
 * \param type          Type of the raw message
 * \param payload       Payload to send
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_send_raw_init(axiom_dev_t *dev, axiom_node_id_t dst, axiom_type_t type,
        axiom_init_payload_t *payload)
{
    axiom_msg_id_t ret;

    ret = axiom_send_raw(dev, dst, AXIOM_RAW_PORT_INIT, type,
            sizeof(*payload), payload);

    DPRINTF("ret: %x payload: %x", ret, (*(uint32_t*)&payload));

    return ret;
}

/*!
 * \brief This function receives an init message
 *
 * \param dev           The axiom device private data pointer
 * \param src           Local interface or remote id identification
 * \param type          Type of the raw message
 * \param cmd           Command of the raw message
 * \param payload_size  Size of payload received
 * \param payload       Payload received
 *
 * \return Returns XXX
 */
inline static axiom_msg_id_t
axiom_recv_raw_init(axiom_dev_t *dev, axiom_node_id_t *src, axiom_type_t *type,
        axiom_init_cmd_t *cmd, axiom_raw_payload_size_t *payload_size,
        axiom_init_payload_t *payload)
{
    axiom_port_t port = AXIOM_RAW_PORT_INIT;
    axiom_msg_id_t ret;

    ret = axiom_recv_raw(dev, src, &port, type, payload_size, payload);

    if ((ret < AXIOM_RET_OK) || (port != AXIOM_RAW_PORT_INIT))
    {
        EPRINTF("ret: %x port: %x type: %x", ret, port, *type);
        return AXIOM_RET_ERROR;
    }

    /* payload info */
    *cmd = payload->command;

    return AXIOM_RET_OK;
}
#endif /* !AXIOM_NIC_INIT_h */
