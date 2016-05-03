#ifndef AXIOM_SWITCH_PACKETS_H
#define AXIOM_SWITCH_PACKETS_H

/*!
 * \file axiom_switch_packets.h
 *
 * \version     v0.4
 * \date        2016-03-15
 *
 * This file contains the AXIOM switch packets used only in the QEMU emulation.
 *
 */

#include "axiom_nic_packets.h"


/********************* Ethernet type for AXIOM switch *************************/

/*!
 * \brief AXIOM control message type
 *
 * This message is used to talk between Axiom NIC emualtion in QEMU and the
 * Axiom switch, exchanging info about the interface (how interface is enabled,
 * etc.)
 */
#define AXIOM_ETH_TYPE_CTRL       0x8332
/*!
 * \brief AXIOM SMALL message type
 *
 * This message is used to exchange Axiom SMALL messages between QEMU VMs and
 * the Axiom switch
 */
#define AXIOM_ETH_TYPE_SMALL      0x8333


/************************ Switch Packets structure ****************************/
/*!
 * \brief Ethernet frame header
 */
typedef struct axiom_eth_hdr {
    uint8_t dhost[6];                   /*!< \brief Destination MAC address */
    uint8_t shost[6];                   /*!< \brief Source MAC address */
    uint16_t type;                      /*!< \brief Ethernet type */
} __attribute__((packed)) axiom_eth_hdr_t;


/*!
 * \brief SMALL packet structure encapsulated in the ethernet frame
 */
typedef struct axiom_small_eth {
    axiom_eth_hdr_t eth_hdr;            /*!< \brief Ethernet frame header */
    axiom_small_msg_t small_msg;        /*!< \brief AXIOM SMALL message */
} __attribute__((packed)) axiom_small_eth_t;



#endif /* !AXIOM_NIC_PACKETS_H */
