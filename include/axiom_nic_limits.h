#ifndef AXIOM_NIC_LIMITS_h
#define AXIOM_NIC_LIMITS_h

/*!
 * \file axiom_nic_limits.h
 *
 * \version     v0.6
 * \date        2016-04-08
 *
 * This file contains the AXIOM limits
 *
 */

/*! \brief Maximum number of nodes supported by the AXIOM NIC */
#define AXIOM_NODES_MAX                         255
/*! \brief Maximum number of interfaces supported by the AXIOM NIC */
#define AXIOM_INTERFACES_MAX                    4
/*! \brief Max number of port available in RAW messages */
#define AXIOM_RAW_PORT_MAX                      8
/*! \brief Max number of type available */
#define AXIOM_TYPE_MAX                          8
/*! \brief Max number of message id available */
#define AXIOM_MSG_ID_MAX                        256

/*! \brief Header size in the raw message */
#define AXIOM_RAW_HEADER_SIZE                   4
/*! \brief Max payload size in the raw message */
#define AXIOM_RAW_PAYLOAD_MAX_SIZE              128

/*! \brief Header size in the rdma message */
#define AXIOM_RDMA_HEADER_SIZE                  13
/*! \brief Max payload size in the rdma message */
#define AXIOM_RDMA_PAYLOAD_MAX_SIZE             524288
/*! \brief Payload size field in the RDMA message is referred to 8-bytes
 *         granularity
 */
#define AXIOM_RDMA_PAYLOAD_ORDER                3

#endif /* !AXIOM_NIC_LIMITS_h */
