#ifndef AXIOM_NIC_PACKETS_HEADER_H
#define AXIOM_NIC_PACKETS_HEADER_H

/*!
 * \file axiom_nic_packets.h
 *
 * \version     v0.5
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

/*! \brief Max number of port available */
#define AXIOM_SMALL_PORT_LENGTH         8


/*************************** AXIOM packet TYPES *******************************/

/*! \brief Axiom type SMALL DATA (contains SMALL data) */
#define AXIOM_TYPE_SMALL_DATA           0
/*! \brief Axiom type SMALL NEIGHBOUR (contains SMALL data to neighbour) */
#define AXIOM_TYPE_SMALL_NEIGHBOUR      1
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

/*! \brief Max number of type available */
#define AXIOM_TYPE_LENGTH               8


/*************************** Packets structure ********************************/

/*! \brief Header size in the small message */
#define AXIOM_SMALL_HEADER_SIZE         4
/*! \brief Max payload size in the small message */
#define AXIOM_SMALL_PAYLOAD_MAX_SIZE    128


/*!
 * \brief Header packet structure for TX small messages
 */
typedef struct axiom_small_tx_hdr {
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
            uint8_t type : 3;
        } field;
    } port_type;                /*!< \brief port and type fields */
    uint8_t src;	        /*!< \brief source (for rx) identificator */
    uint8_t payload_size;       /*!< \brief size of payload */
    uint8_t spare[1];
} axiom_small_rx_hdr_t;


/*!
 * \brief Header packet union for SMALL messages
 */
typedef union axiom_small_hdr {
    axiom_small_tx_hdr_t tx;
    axiom_small_rx_hdr_t rx;
    uint8_t raw[4];
    uint32_t raw32;
} axiom_small_hdr_t;


/*! \brief AXIOM payload type */
typedef struct axiom_payload {
    uint8_t raw[AXIOM_SMALL_PAYLOAD_MAX_SIZE];
} axiom_payload_t;

/*!
 * \brief SMALL messages with payload embedded
 */
typedef struct axiom_small_msg {
    axiom_small_hdr_t header;   /*!< \brief message header */
    axiom_payload_t payload;    /*!< \brief message payload */
} axiom_small_msg_t;

/*!
 * \brief SMALL messages descriptor with a pointer to the payload
 */
typedef struct axiom_small_desc {
    axiom_small_hdr_t header;   /*!< \brief message header */
    void *payload;              /*!< \brief pointer to the message payload */
} axiom_small_desc_t;

#endif /* !AXIOM_NIC_PACKETS_H */
