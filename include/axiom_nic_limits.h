#ifndef AXIOM_NIC_LIMITS_h
#define AXIOM_NIC_LIMITS_h

/*!
 * \file axiom_nic_limits.h
 *
 * \version     v0.5
 * \date        2016-04-08
 *
 * This file contains the AXIOM limits
 *
 */

/*! \brief Maximum number of nodes supported by the AXIOM NIC */
#define AXIOM_NODES_MAX                         255
/*! \brief Maximum number of interfaces supported by the AXIOM NIC */
#define AXIOM_INTERFACES_MAX                    4
/*! \brief Max number of port available in SMALL messages */
#define AXIOM_SMALL_PORT_MAX                    8
/*! \brief Max number of type available */
#define AXIOM_TYPE_MAX                          8

/*! \brief Header size in the small message */
#define AXIOM_SMALL_HEADER_SIZE                 4
/*! \brief Max payload size in the small message */
#define AXIOM_SMALL_PAYLOAD_MAX_SIZE            128


#endif /* !AXIOM_NIC_LIMITS_h */
