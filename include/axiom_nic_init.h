#ifndef AXIOM_NIC_INIT_h
#define AXIOM_NIC_INIT_h

/*
 * axiom_nic_init.h
 *
 * Version:     v0.3.1
 * Last update: 2016-04-13
 *
 * This file contains the AXIOM NIC types for axiom_init
 *
 */
#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_small_commands.h"

/********************************* Types **************************************/
typedef uint8_t		axiom_init_cmd_t;	/* Discovery command */



/********************************* Packet *************************************/
typedef struct axiom_ping_payload {
    uint8_t  command;                    /* Command of ping-pong messages */
    uint16_t packet_id;                  /* Packet id */
    uint8_t  spare;                      /* Spare */
} axiom_ping_payload_t;

#endif /* !AXIOM_NIC_INIT_h */
