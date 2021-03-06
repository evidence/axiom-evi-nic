/*!
 * \file axiom_nic_limits.h
 *
 * \version     v1.2
 * \date        2016-04-08
 *
 * This file contains the AXIOM limits
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_LIMITS_h
#define AXIOM_NIC_LIMITS_h

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/*! \brief Maximum value of node_id supported by the AXIOM NIC */
#define AXIOM_NODES_MAX                         255
/*! \brief Maximum number of node_id supported by the AXIOM NIC */
#define AXIOM_NODES_NUM                         (AXIOM_NODES_MAX + 1)
/*! \brief Maximum value of interfaces supported by the AXIOM NIC */
#define AXIOM_INTERFACES_MAX                    4
/*! \brief Maximum number of interfaces supported by the AXIOM NIC */
#define AXIOM_INTERFACES_NUM                    (AXIOM_INTERFACES_MAX + 1)
/*! \brief Max value of port available in RAW messages.
 *         Note: port 7 is reserved for XSMLL messages */
#define AXIOM_PORT_MAX                          6
/*! \brief Max number of port available in RAW messages. */
#define AXIOM_PORT_NUM                          (AXIOM_PORT_MAX + 1)
/*! \brief Max value of type available */
#define AXIOM_TYPE_MAX                          7
/*! \brief Max number of type available */
#define AXIOM_TYPE_NUM                          (AXIOM_TYPE_MAX + 1)
/*! \brief Max value of message id available */
#define AXIOM_MSG_ID_MAX                        255
/*! \brief Max number of message id available */
#define AXIOM_MSG_ID_NUM                        (AXIOM_MSG_ID_MAX + 1)

/*! \brief Header size in the raw message */
#define AXIOM_RAW_HEADER_SIZE                   5
/*! \brief Padding (bytes) in the raw message */
#define AXIOM_RAW_PADDING                       3
/*! \brief Max payload size in the raw message */
#define AXIOM_RAW_PAYLOAD_MAX_SIZE              248

/*! \brief Header size (bytes) in the rdma message */
#define AXIOM_RDMA_HEADER_SIZE                  13
/*! \brief Padding (bytes) in the rdma message */
#define AXIOM_RDMA_PADDING                      3
/*! \brief Max payload size (bytes) in the rdma message */
#define AXIOM_RDMA_PAYLOAD_MAX_SIZE             524280
/*! \brief Payload size field in the RDMA message is referred to 8-bytes
 *         granularity
 */
#define AXIOM_RDMA_PAYLOAD_SIZE_ORDER           3
/*! \brief RDMA address alignement requested */
#define AXIOM_RDMA_ADDRESS_ALIGNMENT            8

/*! \brief Max payload size in the long message */
#define AXIOM_LONG_PAYLOAD_MAX_SIZE             65528

/*! \brief  Max memory segment size */
#define AXIOM_MAX_SEGMENT_SIZE                  (2L*1024*1024*1204)

/** \} */

#endif /* !AXIOM_NIC_LIMITS_h */
