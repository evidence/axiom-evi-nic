#ifndef AXIOM_NIC_PACKETS_HEADER_H
#define AXIOM_NIC_PACKETS_HEADER_H

/*
 * axiom_nic_packets.h
 *
 * Version:     v0.2
 * Last update: 2016-03-25
 *
 * This file contains the following AXIOM NIC packets description:
 *      - SMALL data packet
 *      - SMALL to neighbour
 *
 */

/*************************** Packets structure ********************************/

#define AXIOM_SMALL_PORT_DISCOVERY      0
#define AXIOM_SMALL_PORT_ROUTING        0

#define AXIOM_SMALL_FLAG_NEIGHBOUR      0x1

/*
 * Header packet structure
 */
typedef struct axiom_small_tx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t flag : 3;
        } field;
    } port_flag;                        /* port and flag fields */
    uint8_t dst;	                /* destination (for tx) identificator */
    uint8_t spare[2];
} axiom_small_tx_hdr_t;

typedef struct axiom_small_rx_hdr {
    union {
        uint8_t raw;
        struct {
            uint8_t reserved : 2;
            uint8_t port : 3;
            uint8_t flag : 3;
        } field;
    } port_flag;                        /* port and flag fields */
    uint8_t src;	                /* source (for rx) identificator */
    uint8_t spare[2];
} axiom_small_rx_hdr_t;

/*
 * SMALL packet structure
 */
typedef struct axiom_small_msg {
    union {
        axiom_small_tx_hdr_t tx;
        axiom_small_rx_hdr_t rx;
	} header;                       /* Message header */
    uint32_t payload;	                /* Data to be sent */
} axiom_small_msg_t;


#endif /* !AXIOM_NIC_PACKETS_H */
