#ifndef AXIOM_NIC_INIT_h
#define AXIOM_NIC_INIT_h

/*!
 * \file axiom_nic_init.h
 *
 * \version     v0.8
 * \date        2016-04-13
 *
 * This file contains the AXIOM NIC types for axiom-init deamon
 *
 */
#include "dprintf.h"
#include "axiom_nic_limits.h"
#include "axiom_nic_types.h"
#include "axiom_nic_raw_commands.h"
#include "axiom_global_allocator.h"

/*! \brief axiom master node */
#define AXIOM_MASTER_NODE_INIT          0

/********************************* Types **************************************/
typedef uint8_t		axiom_init_cmd_t;   /*!< \brief init command type*/

typedef enum {
    AXNP_RDMA = 0x001,
    AXNP_LONG = 0x002,
    AXNP_RAW = 0x004
} axiom_netperf_type_t;                     /*!< \brief axiom netperf type */


/********************************* Packets *************************************/

/*! \brief Generic message payload for the axiom-init deamon */
typedef struct axiom_init_payload {
    uint8_t  command;           /*!< \brief Command of messages */
    uint8_t  payload[127];
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


/*! \brief Message payload for the axiom allocator */
typedef struct axiom_allocator_payload {
    uint8_t  command;           /*!< \brief Command of allocator messages */
    uint8_t  reply_port;        /*!< \brief Port where to reply */
    uint8_t  padding[5];
    axiom_galloc_info_t info;
} axiom_allocator_payload_t;


/* flags fod spawn messages */

/*! \brief Reset the given spawn session to default values */
#define AXIOM_SPAWN_FLAG_RESET 0x01
/*! \brief Run the application described into the given session */
#define AXIOM_SPAWN_FLAG_EXEC  0x02

/* type of messagw spawn */

/*! \brief Data contains exec filename */
#define AXIOM_SPAWN_TYPE_EXE 0
/*! \brief Data contains one arg */
#define AXIOM_SPAWN_TYPE_ARG 1
/*! \brief Data contains one environemtn variable */
#define AXIOM_SPAWN_TYPE_ENV 2
/*! \brief Data contains working directory */
#define AXIOM_SPAWN_TYPE_CWD 3

/*! \brief Size of spwan message header */
#define AXIOM_SPAWN_HEADER_SIZE 4
/*! \brief Max size of data into spawn messages */
#define AXIOM_SPAWN_MAX_DATA_SIZE (AXIOM_RAW_PAYLOAD_MAX_SIZE-AXIOM_SPAWN_HEADER_SIZE)

/*! \brief Message payload for the axiom-netperf application */
typedef struct axiom_spawn_req_payload {
    uint8_t command; /*!< \brief Command of spawn request messages */
    uint8_t flags; /*!< \brief Flags */
    uint8_t session_id; /*!< \brief Session identification */
    uint8_t type; /*!< \brief Type */
    uint8_t data[AXIOM_SPAWN_MAX_DATA_SIZE]; /*!< \brief Data for type */
} __attribute__((__packed__)) axiom_spawn_req_payload_t;

/*! \brief Message payload for the axiom request session */
typedef struct axiom_session_req_payload {
    uint8_t command; /*!< \brief Command of session request messages */
    uint8_t reply_port; /*!< \brief Port used to reply message */
    uint8_t session_id; /*!< \brief Session id */
    uint8_t spare[1]; /*!< \brief Don't care */
} __attribute__((__packed__)) axiom_session_req_payload_t;

/*! \brief Indicate an empty (no valid) session */
#define AXIOM_SESSION_EMPTY 255

/*! \brief Message payload for the axiom reply session */
typedef struct axiom_session_reply_payload {
    uint8_t command; /*!< \brief Command of session request messages */
    uint8_t session_id; /*!< \brief Session id */
    uint8_t spare[2]; /*!< \brief Don't care */
} __attribute__((__packed__)) axiom_session_reply_payload_t;

/******************************* Functions ************************************/

/*!
 * \brief This function receives the init messages
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
inline static axiom_err_t
axiom_recv_init(axiom_dev_t *dev, axiom_node_id_t *src, axiom_type_t *type,
        axiom_init_cmd_t *cmd, size_t *payload_size, void *payload)
{
    axiom_port_t port = AXIOM_RAW_PORT_INIT;
    axiom_msg_id_t ret;

    ret = axiom_recv(dev, src, &port, type, payload_size, payload);

    if ((ret < AXIOM_RET_OK) || (port != AXIOM_RAW_PORT_INIT) ||
            (*payload_size == 0))
    {
        EPRINTF("ret: %x port: %x type: %x", ret, port, *type);
        return AXIOM_RET_ERROR;
    }

    /* payload info */
    *cmd = ((axiom_init_payload_t *)payload)->command;

    return AXIOM_RET_OK;
}
#endif /* !AXIOM_NIC_INIT_h */
