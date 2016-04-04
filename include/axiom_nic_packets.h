#ifndef AXIOM_NIC_PACKETS_H
#define AXIOM_NIC_PACKETS_H

/*
 * axiom_nic_packets.h
 *
 * Last update: 2016-03-08
 *
 * This file contains the following AXIOM NIC packets description:
 *      - RAW packet
 *      - RAW to neighbour
 *      - RDMA
 *      - discovery (RAW to neighbour)
 *      - routing (RAW)
 *
 */

/*************************** Packets structure ********************************/

/* RAW message type */
#define AXIOM_RAW_TYPE_GENERIC		0
#define AXIOM_RAW_TYPE_DISCOVERY	1
#define AXIOM_RAW_TYPE_ROUTING      2

/* RAW message flags */
#define AXIOM_RAW_FLAG_NEIGHBOUR        0x01
#define AXIOM_RAW_FLAG_OVERFLOW         0x02

/*
 * Header packet structure
 */
typedef struct axiom_raw_hdr {
	uint8_t src_node;	        /* Sender node id */
	uint8_t dst_node;	        /* Receiver node id */
	uint8_t flags;                  /* Raw flags (TODO: add size ?)*/
	uint8_t type;                   /* Raw type */
} axiom_raw_hdr_t;

typedef struct axiom_neighbour_hdr {
	uint8_t src_if;	                /* Sender interface */
	uint8_t dst_if;	                /* Receiver interface (filled by receiver) */
	uint8_t flags;                  /* Raw flags (NEIGHBOUR flag set) */
	uint8_t type;                   /* Raw type */
} axiom_neighbour_hdr_t;

/*
 * RAW packet structure
 */
typedef struct axiom_raw_msg {
        union {
	    axiom_raw_hdr_t raw;
	    axiom_neighbour_hdr_t neighbour;
	} header;                       /* Message header */
	union {
	    uint32_t raw;	        /* Data to be sent */
	} data;
} axiom_raw_msg_t;


/*
 * RDMA packet structure
 */
typedef struct axiom_rdma_msg {
	uint8_t src_node;               /* Sender node id */
	uint8_t dst_node;	        /* Receiver node id */
	uint16_t payload_size;	        /* Size of payload */
	uint32_t *src_addr;	        /* Starting address of payload */
	uint32_t *dst_addr;	        /* Payload destination address */
} axiom_rdma_msg_t;





/***********************************************************************/
/* ********************** Neighbours packets ************************* */
/***********************************************************************/
/*
 * Discovery packet structure (AXIOM_RAW_TYPE_DISCOVERY)
*/

#define AXIOM_DSCV_TYPE_REQ_ID		0 /* Request the  ID */
#define AXIOM_DSCV_TYPE_RSP_NOID	1 /* Response with no ID */
#define AXIOM_DSCV_TYPE_RSP_ID		2 /* Response with my ID */
#define AXIOM_DSCV_TYPE_SETID		3 /* Request to set ID */
#define AXIOM_DSCV_TYPE_START		4 /* Request to start discovery */
#define AXIOM_DSCV_TYPE_TOPOLOGY	5 /* The message contains a topology
					   * row
					   */
#define AXIOM_DSCV_TYPE_END_TOPOLOGY	6 /* The message says that the
                       * last AXIOM_DSCV_TYPE_TOPOLOGY message received
                       * was the last topology row
					   */
typedef struct axiom_discovery_data {
    uint8_t type;                       /* Type of discovery messages */
    uint8_t src_node;                   /* Source node id */
    uint8_t dst_node;                   /* Destination node id */
    uint8_t src_dst_if;                 /* Source interface | Dest Interface */
} axiom_discovery_data_t;


typedef struct axiom_discovery_msg {
    union {
	axiom_neighbour_hdr_t neighbour;
    } header;
    union {
        axiom_discovery_data_t disc;
        uint32_t raw;
    } data;
} axiom_discovery_msg_t;


/*
 * Set Routing packet structure (AXIOM_RAW_TYPE_ROUTING)
*/
#define AXIOM_RT_TYPE_SET_ROUTING	7 /* Request to set the routing
                       * table delivered by Master node*/
typedef struct axiom_set_routing_data {
    uint8_t type;                 /* Type of set routing messages */
    uint8_t spare1;
    uint8_t spare2;
    uint8_t spare3;
} axiom_set_routing_data_t;

typedef struct axiom_set_routing_msg {
    union {
    axiom_neighbour_hdr_t neighbour;
    } header;
    union {
        axiom_set_routing_data_t set_routing;
        uint32_t raw;
    } data;
} axiom_set_routing_msg_t;


/***********************************************************************/
/* ************************** Raw packets **************************** */
/***********************************************************************/
/*
 * Routing tables delivery packet structure (AXIOM_RAW_TYPE_ROUTING)
*/

#define AXIOM_RT_TYPE_INFO          0 /* send a (node_id,if_) pair */
#define AXIOM_RT_TYPE_END_INFO      1 /* end of a node routing table */

typedef struct axiom_rt_delivery_data {
    uint8_t type;               /* Type of discovery messages */
    uint8_t node_id;            /* ID of the node to set  into the routing
                                   table */
    uint8_t if_id;              /* ID of the interface of node_id node*/
    uint8_t spare;
} axiom_rt_delivery_data_t;

typedef struct axiom_rt_delivery_msg {
    union {
	axiom_raw_hdr_t raw;
    } header;                       /* Message header */
    union {
        axiom_rt_delivery_data_t r_table;
        uint32_t raw;
    } data;
} axiom_rt_delivery_msg_t;

#endif /* !AXIOM_NIC_PACKETS_H */
