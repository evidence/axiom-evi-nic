#ifndef AXIOM_NIC_RAW_COMMANDS_h
#define AXIOM_NIC_RAW_COMMANDS_h

/*!
 * \file axiom_nic_raw_commands.h
 *
 * \version     v0.5
 * \date        2016-03-22
 *
 * This file contains the AXIOM raw messages commands
 *
 */

/*! \brief Discovery phase: Request the node ID */
#define AXIOM_DSCV_CMD_REQ_ID           0
/*! \brief Discovery phase: Response with no ID */
#define AXIOM_DSCV_CMD_RSP_NOID         1
/*! \brief Discovery phase: Response with local ID */
#define AXIOM_DSCV_CMD_RSP_ID           2
/*! \brief Discovery phase: Request to set ID */
#define AXIOM_DSCV_CMD_SETID            3
/*! \brief Discovery phase: Request to start discovery */
#define AXIOM_DSCV_CMD_START            4
/*! \brief Discovery phase: The message contains a topology row */
#define AXIOM_DSCV_CMD_TOPOLOGY         5
/*! \brief Discovery phase: End of topology table */
#define AXIOM_DSCV_CMD_END_TOPOLOGY     6

/*! \brief Routing phase: Start to use a new rotuing table delivered by the master */
#define AXIOM_RT_CMD_SET_ROUTING        7
/*! \brief Routing phase: Send routing table row (node_id, if_mask)*/
#define AXIOM_RT_CMD_INFO               8
/*! \brief Routing phase: End of routing table */
#define AXIOM_RT_CMD_END_INFO           9
/*! \brief Routing phase: Confirm routing table reception*/
#define AXIOM_RT_CMD_RT_REPLY          10

/*! \brief Network utilities: ping request */
#define AXIOM_CMD_PING                 11
/*! \brief Network utilities: ping reply */
#define AXIOM_CMD_PONG                 12
/*! \brief Network utilities: traceroute request */
#define AXIOM_CMD_TRACEROUTE           13
/*! \brief Network utilities: traceroute replay*/
#define AXIOM_CMD_TRACEROUTE_REPLY     14
/*! \brief Network utilities: netperf payload packet */
#define AXIOM_CMD_NETPERF              15
/*! \brief Network utilities: netperf start */
#define AXIOM_CMD_NETPERF_START        16
/*! \brief Network utilities: netperf end */
#define AXIOM_CMD_NETPERF_END          17




#endif /* !AXIOM_NIC_RAW_COMMANDS_h */
