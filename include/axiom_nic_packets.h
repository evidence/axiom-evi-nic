#ifndef AXIOM_NIC_PACKETS_HEADER_H
#define AXIOM_NIC_PACKETS_HEADER_H

/*!
 * \file axiom_nic_packets.h
 *
 * \version     v0.4
 * \date        2016-03-25
 *
 * This file contains the following AXIOM NIC packets description:
 *      - SMALL data packet
 *      - SMALL to neighbour
 *
 */


/************************* Well-known AXIOM port ******************************/

/*! \brief axiom-init deamon port number */
#define AXIOM_SMALL_PORT_INIT           0
/*! \brief Axiom network utilities port number */
#define AXIOM_SMALL_PORT_NETUTILS       1


/*************************** AXIOM SMALL FLAGS ********************************/

/*! \brief Axiom flag DATA (message contains RAW data) */
#define AXIOM_SMALL_FLAG_DATA           0x0
/*! \brief Axiom flag NEIGHBOUR (message contains RAW data to neighbour) */
#define AXIOM_SMALL_FLAG_NEIGHBOUR      0x1
/*! \brief Axiom flag ACK (message contains an ACK) */
#define AXIOM_SMALL_FLAG_ACK            0x2


/*************************** Packets structure ********************************/

/*!
 * \brief Header packet structure for TX small messages
 */
typedef struct axiom_small_tx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t flag : 3;
        } field;
    } port_flag;        /*!< \brief port and flag fields */
    uint8_t dst;	/*!< \brief destination (for tx) identificator */
    uint8_t spare[2];
} axiom_small_tx_hdr_t;

/*!
 * \brief Header packet structure for RX small messages
 */
typedef struct axiom_small_rx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t flag : 3;
        } field;
    } port_flag;        /*!< \brief port and flag fields */
    uint8_t src;	/*!< \brief source (for rx) identificator */
    uint8_t spare[2];
} axiom_small_rx_hdr_t;

/*!
 * \brief SMALL messages structure
 */
typedef struct axiom_small_msg {
    union {
        axiom_small_tx_hdr_t tx;
        axiom_small_rx_hdr_t rx;
	} header;       /*!< \brief message header */
    uint32_t payload;	/*!< \brief message payload */
} axiom_small_msg_t;


#endif /* !AXIOM_NIC_PACKETS_H */
