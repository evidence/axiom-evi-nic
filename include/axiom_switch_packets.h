#ifndef AXIOM_SWITCH_PACKETS_H
#define AXIOM_SWITCH_PACKETS_H

/*!
 * \file axiom_switch_packets.h
 *
 * \version     v0.6
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
#define AXIOM_ETH_TYPE_CTRL             0x8332
/*!
 * \brief AXIOM RAW message type
 *
 * This message is used to exchange Axiom RAW messages between QEMU VMs and
 * the Axiom switch
 */
#define AXIOM_ETH_TYPE_RAW              0x8333
/*!
 * \brief AXIOM RDMA message type
 *
 * This message is used to exchange Axiom RDMA messages between QEMU VMs and
 * the Axiom switch
 */
#define AXIOM_ETH_TYPE_RDMA             0x8334


/********************** Messages for the AXIOM switch *************************/

/*!
 * \brief control packet structure
 */
typedef struct axiom_ctrl_msg {
    uint8_t if_id;                      /*!< \brief Interface ID */
    uint8_t if_info;                    /*!< \brief Interface info */
} axiom_ctrl_msg_t;


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
 * \brief RAW packet structure encapsulated in the ethernet frame
 */
typedef struct axiom_eth_raw {
    axiom_eth_hdr_t eth_hdr;            /*!< \brief Ethernet frame header */
    axiom_raw_msg_t raw_msg;            /*!< \brief AXIOM RAW message */
} __attribute__((packed)) axiom_eth_raw_t;

/*!
 * \brief RDMA packet structure encapsulated in the ethernet frame
 */
typedef struct axiom_eth_rdma {
    axiom_eth_hdr_t eth_hdr;            /*!< \brief Ethernet frame header */
    axiom_rdma_hdr_t rdma_hdr;          /*!< \brief AXIOM RDMA header */
    /*! \brief AXIOM RDMA payload */
    uint8_t rdma_payload[AXIOM_RDMA_PAYLOAD_MAX_SIZE];
} __attribute__((packed)) axiom_eth_rdma_t;

/*!
 * \brief control packet structure encapsulated in the ethernet frame
 */
typedef struct axiom_eth_ctrl {
    axiom_eth_hdr_t eth_hdr;            /*!< \brief Ethernet frame header */
    axiom_ctrl_msg_t ctrl_msg;          /*!< \brief AXIOM control message */
} __attribute__((packed)) axiom_eth_ctrl_t;

/*!
 * \brief axiom packet encapsulated in the ethernet frame
 */
typedef union axiom_eth_pkt {
    axiom_eth_hdr_t eth_hdr;            /*!< \brief Ethernet frame header */
    axiom_eth_raw_t raw;                /*!< \brief AXIOM RAW packet */
    axiom_eth_rdma_t rdma;              /*!< \brief AXIOM RDMA packet */
    axiom_eth_ctrl_t ctrl;              /*!< \brief AXIOM control packet */
} axiom_eth_pkt_t;
#endif /* !AXIOM_NIC_PACKETS_H */
