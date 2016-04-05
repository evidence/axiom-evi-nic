#ifndef AXIOM_SWITCH_PACKETS_H
#define AXIOM_SWITCH_PACKETS_H

/*
 * axiom_switch_packets.h
 *
 * Version:     v0.3
 * Last update: 2016-03-15
 *
 * This file contains the AXIOM switch packets description
 *
 */

#include "axiom_nic_packets.h"

/************************ Switch Packets structure ****************************/

#define AXIOM_ETH_TYPE_CTRL       0x8332
#define AXIOM_ETH_TYPE_SMALL      0x8333

/*
 * Ethernet header
 */
typedef struct axiom_eth_hdr {
    uint8_t dhost[6];
    uint8_t shost[6];
    uint16_t type;
} __attribute__((packed)) axiom_eth_hdr_t;


/*
 * SMALL packet structure encapsuled in the ethernet frame
 */
typedef struct axiom_small_eth {
    axiom_eth_hdr_t eth_hdr;
    axiom_small_msg_t small_msg;
} __attribute__((packed)) axiom_small_eth_t;



#endif /* !AXIOM_NIC_PACKETS_H */
