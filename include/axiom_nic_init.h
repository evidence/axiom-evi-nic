#ifndef AXIOM_NIC_INIT_h
#define AXIOM_NIC_INIT_h

/*
 * axiom_nic_init.h
 *
 * Version:     v0.4
 * Last update: 2016-04-13
 *
 * This file contains the AXIOM NIC types for axiom_init
 *
 */
#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_small_commands.h"

/********************************* Types **************************************/
typedef uint8_t		axiom_init_cmd_t;	/* init command */



/********************************* Packets *************************************/
typedef struct axiom_init_payload {
    uint8_t  command;                    /* Command of messages */
    uint8_t  spare[3];
} axiom_init_payload_t;


typedef struct axiom_ping_payload {
    uint8_t  command;                    /* Command of ping-pong messages */
    uint8_t  spare;                      /* Spare */
    uint16_t packet_id;                  /* Packet id */
} axiom_ping_payload_t;


typedef struct axiom_traceroute_payload {
    uint8_t  command;                    /* Command of traceroute messages */
    uint8_t  src_id;                     /* Source node id */
    uint8_t  dst_id;                     /* Destination node id */
    uint8_t  step;                       /* Step of the message route */
} axiom_traceroute_payload_t;


typedef struct axiom_netperf_payload {
    uint8_t  command;                    /* Command of netperf messages */
    uint8_t  offset;                     /* Packet offset */
    uint16_t data;                       /* Packet data */
} axiom_netperf_payload_t;

/*
 * @brief This function sends a init message.
 *
 * @param dev           The axiom devive private data pointer
 * @param dst           Local interface or remote id identification
 * @param flag          flags of the small message
 * @param payload       payload to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_small_init(axiom_dev_t *dev, axiom_node_id_t dst,
        axiom_flag_t flag, axiom_payload_t *payload)
{
    axiom_msg_id_t ret;

    ret = axiom_send_small(dev, dst, AXIOM_SMALL_PORT_INIT, flag, payload);

    DPRINTF("ret: %x payload: %x", ret, (*(uint32_t*)&payload));

    return ret;
}

/*
 * @brief This function receive a init message
 *
 * @param dev           The axiom devive private data pointer
 * @param src           Local interface or remote id identification
 * @param flag          flags of the small message
 * @param cmd           command of the small message
 * @param payload       payload received
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_recv_small_init(axiom_dev_t *dev, axiom_node_id_t *src, axiom_flag_t *flag,
        axiom_init_cmd_t *cmd, axiom_payload_t *payload)
{
    axiom_port_t port = AXIOM_SMALL_PORT_INIT;
    axiom_msg_id_t ret;
    axiom_init_payload_t *init_payload;

    ret = axiom_recv_small(dev, src, &port, flag, payload);

    if ((ret != AXIOM_RET_OK) || (port != AXIOM_SMALL_PORT_INIT))
    {
        EPRINTF("ret: %x port: %x flag: %x payload: %x", ret, port, *flag,
                (*payload));
        return AXIOM_RET_ERROR;
    }

    /* payload info */
    init_payload = ((axiom_init_payload_t *)payload);
    *cmd = init_payload->command;

    return AXIOM_RET_OK;
}
#endif /* !AXIOM_NIC_INIT_h */
