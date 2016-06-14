#ifndef AXIOM_NIC_PACKETS_HEADER_H
#define AXIOM_NIC_PACKETS_HEADER_H

/*!
 * \file axiom_nic_packets.h
 *
 * \version     v0.6
 * \date        2016-03-25
 *
 * This file contains the following AXIOM NIC packets description:
 *      - RAW data packet
 *      - RAW to neighbour
 *
 */
#include "axiom_nic_limits.h"


/************************* Well-known AXIOM port ******************************/

/*! \brief axiom-init deamon port number */
#define AXIOM_RAW_PORT_INIT             0
/*! \brief Axiom network utilities port number */
#define AXIOM_RAW_PORT_NETUTILS         1


/*************************** AXIOM packet TYPES *******************************/

/*! \brief Axiom type RAW DATA (contains RAW data) */
#define AXIOM_TYPE_RAW_DATA             0
/*! \brief Axiom type RAW NEIGHBOUR (contains RAW data to neighbour) */
#define AXIOM_TYPE_RAW_NEIGHBOUR        1
/*! \brief Axiom type LONG DATA (contains LONG data) */
#define AXIOM_TYPE_LONG_DATA            2
/*! \brief Axiom type RDMA WRITE (contains RDMA write) */
#define AXIOM_TYPE_RDMA_WRITE           3
/*! \brief Axiom type RDMA REQ (contains RDMA request) */
#define AXIOM_TYPE_RDMA_REQ             4
/*! \brief Axiom type RDMA RESPONSE (contains RDMA response - HW reserved) */
#define AXIOM_TYPE_RDMA_RESPONE         5
/*! \brief Axiom type INIT (contains an INIT - HW reserved) */
#define AXIOM_TYPE_INIT                 6
/*! \brief Axiom type ACK (contains an ACK - HW reserved) */
#define AXIOM_TYPE_ACK                  7


/*************************** Packets structure ********************************/

/*!
 * \brief Header packet structure for TX raw messages
 */
typedef struct axiom_raw_tx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t type : 3;
        } field;
    } port_type;                /*!< \brief port and type fields */
    uint8_t dst;	        /*!< \brief destination (for tx) identificator*/
    uint8_t payload_size;       /*!< \brief size of payload */
    uint8_t spare[1];
} axiom_raw_tx_hdr_t;

/*!
 * \brief Header packet structure for RX raw messages
 */
typedef struct axiom_raw_rx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t type : 3;
        } field;
    } port_type;                /*!< \brief port and type fields */
    uint8_t src;	        /*!< \brief source (for rx) identificator */
    uint8_t payload_size;       /*!< \brief size of payload */
    uint8_t spare[1];
} axiom_raw_rx_hdr_t;


/*!
 * \brief Header packet union for RAW messages
 */
typedef union axiom_raw_hdr {
    axiom_raw_tx_hdr_t tx;
    axiom_raw_rx_hdr_t rx;
    uint8_t raw[4];
    uint32_t raw32;
} axiom_raw_hdr_t;


/*! \brief AXIOM payload type */
typedef struct axiom_payload {
    uint8_t raw[AXIOM_RAW_PAYLOAD_MAX_SIZE];
} axiom_payload_t;

/*!
 * \brief RAW messages with payload embedded
 */
typedef struct axiom_raw_msg {
    axiom_raw_hdr_t header;     /*!< \brief message header */
    axiom_payload_t payload;    /*!< \brief message payload */
} axiom_raw_msg_t;

#endif /* !AXIOM_NIC_PACKETS_H */
