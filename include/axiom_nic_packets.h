/*!
 * \file axiom_nic_packets.h
 *
 * \version     v0.15
 * \date        2016-03-25
 *
 * This file contains the following AXIOM NIC packets description:
 *      - RAW data packet
 *      - RAW to neighbour
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_PACKETS_HEADER_H
#define AXIOM_NIC_PACKETS_HEADER_H

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

#include "axiom_nic_limits.h"

/************************* Well-known AXIOM port ******************************/

/*! \brief axiom-init deamon port number */
#define AXIOM_RAW_PORT_INIT             0
/*! \brief Axiom network utilities port number */
#define AXIOM_RAW_PORT_NETUTILS         1


/*************************** AXIOM packet TYPES *******************************/

/*! \brief Axiom type ACK (contains an ACK - HW reserved) */
#define AXIOM_TYPE_ACK                  0x0
/*! \brief Axiom type INIT (contains an INIT - HW reserved) */
#define AXIOM_TYPE_INIT                 0x1
/*! \brief Axiom type RAW NEIGHBOUR (contains RAW data to neighbour) */
#define AXIOM_TYPE_RAW_NEIGHBOUR        0x2
/*! \brief Axiom type RAW DATA (contains RAW data) */
#define AXIOM_TYPE_RAW_DATA             0x3
/*! \brief Axiom type LONG DATA (contains LONG data) */
#define AXIOM_TYPE_LONG_DATA            0x4
/*! \brief Axiom type RDMA WRITE (contains RDMA write) */
#define AXIOM_TYPE_RDMA_WRITE           0x5
/*! \brief Axiom type RDMA READ (contains RDMA read) */
#define AXIOM_TYPE_RDMA_READ            0x6
/*! \brief Axiom type RDMA RESPONSE (contains RDMA response - HW reserved) */
#define AXIOM_TYPE_RDMA_RESPONSE        0x7


/*!
 * \brief AXIOM port/type for RAW and RDMA messages
 */
typedef union axiom_port_type {
    uint8_t raw;
    struct {
        uint8_t type : 3;
        uint8_t port : 3;
        uint8_t error : 1;
        uint8_t s : 1;
    } field;
} __attribute__((packed)) axiom_port_type_t;

/************************* RAW Packets structure ******************************/

/*!
 * \brief Header packet structure for TX raw messages
 */
typedef struct axiom_raw_tx_hdr {
    axiom_port_type_t port_type;/*!< \brief port and type fields */
    uint8_t dst;	        /*!< \brief destination (for tx) identificator*/
    uint8_t msg_id;             /*!< \brief message unique id */
    uint8_t payload_size;       /*!< \brief size of payload */
    uint8_t padding;            /*!< \brief padding unused */
} __attribute__((packed)) axiom_raw_tx_hdr_t;

/*!
 * \brief Header packet structure for RX raw messages
 */
typedef struct axiom_raw_rx_hdr {
    axiom_port_type_t port_type;/*!< \brief port and type fields */
    uint8_t src;	        /*!< \brief source (for rx) identificator */
    uint8_t msg_id;             /*!< \brief message unique id */
    uint8_t payload_size;       /*!< \brief size of payload */
    uint8_t padding;            /*!< \brief padding unused */
} __attribute__((packed)) axiom_raw_rx_hdr_t;


/*!
 * \brief Header packet union for RAW messages
 */
typedef union axiom_raw_hdr {
    axiom_raw_tx_hdr_t tx;
    axiom_raw_rx_hdr_t rx;
    uint8_t raw[AXIOM_RAW_HEADER_SIZE];
} __attribute__((packed)) axiom_raw_hdr_t;

/*! \brief AXIOM RAW payload type */
typedef struct axiom_raw_payload {
    uint8_t raw[AXIOM_RAW_PAYLOAD_MAX_SIZE + AXIOM_RAW_PADDING];
} __attribute__((packed)) axiom_raw_payload_t;

/*!
 * \brief RAW messages with embedded payload
 */
typedef struct axiom_raw_msg {
    axiom_raw_hdr_t header;             /*!< \brief message header */
    axiom_raw_payload_t payload;        /*!< \brief message payload */
} __attribute__((packed)) axiom_raw_msg_t;


/************************* RDMA Packets structure *****************************/

/*!
 * \brief Header packet structure for TX RDMA messages
 */
typedef struct axiom_rdma_tx_hdr {
    axiom_port_type_t port_type;/*!< \brief port and type fields */
    uint8_t dst;	        /*!< \brief destination (for tx) identificator*/
    uint8_t msg_id;             /*!< \brief message unique id */
    uint16_t payload_size;      /*!< \brief size of payload */
    uint32_t src_addr;          /*!< \brief source address of payload */
    uint32_t dst_addr;          /*!< \brief destination address of payload */
    uint8_t padding[3];         /*!< \brief padding unused */
} __attribute__((packed)) axiom_rdma_tx_hdr_t;

/*!
 * \brief Header packet structure for RX RDMA messages
 */
typedef struct axiom_rdma_rx_hdr {
    axiom_port_type_t port_type;/*!< \brief port and type fields */
    uint8_t src;	        /*!< \brief source (for rx) identificator */
    uint8_t msg_id;             /*!< \brief message unique id */
    uint16_t payload_size;      /*!< \brief size of payload */
    uint32_t dst_addr;          /*!< \brief destination address of payload */
    uint8_t padding[7];         /*!< \brief padding unused */
} __attribute__((packed)) axiom_rdma_rx_hdr_t;

/*!
 * \brief Header packet union for RDMA messages
 */
typedef union axiom_rdma_hdr {
    axiom_rdma_tx_hdr_t tx;
    axiom_rdma_rx_hdr_t rx;
    uint8_t raw[AXIOM_RDMA_HEADER_SIZE + AXIOM_RDMA_PADDING];
} __attribute__((packed)) axiom_rdma_hdr_t;

/*!
 * \brief LONG messages with payload pointer
 */
typedef struct axiom_long_msg {
    axiom_rdma_hdr_t header;            /*!< \brief message header */
    void *payload;                      /*!< \brief message payload */
} __attribute__((packed)) axiom_long_msg_t;

/*! \brief AXIOM LONG payload type */
typedef struct axiom_long_payload {
    uint8_t raw[AXIOM_LONG_PAYLOAD_MAX_SIZE];
} __attribute__((packed)) axiom_long_payload_t;

/** \} */

#endif /* !AXIOM_NIC_PACKETS_H */
