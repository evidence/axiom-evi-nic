#ifndef AXIOM_NIC_SMALL_COMMANDS_h
#define AXIOM_NIC_SMALL_COMMANDS_h

/*
 * axiom_nic_small_commands.h
 *
 * Version:     v0.3.1
 * Last update: 2016-03-22
 *
 * This file contains the AXIOM NIC commands
 * for the small messages
 *
 */


#define AXIOM_DSCV_CMD_REQ_ID           0 /* Request the  ID */
#define AXIOM_DSCV_CMD_RSP_NOID         1 /* Response with no ID */
#define AXIOM_DSCV_CMD_RSP_ID           2 /* Response with my ID */
#define AXIOM_DSCV_CMD_SETID            3 /* Request to set ID */
#define AXIOM_DSCV_CMD_START            4 /* Request to start discovery */
#define AXIOM_DSCV_CMD_TOPOLOGY         5 /* The message contains a topology
                                           * row
					   */
#define AXIOM_DSCV_CMD_END_TOPOLOGY     6 /* The message says that the last
                                           * AXIOM_DSCV_CMD_TOPOLOGY message
                                           * received was the last topology row
					   */
#define AXIOM_RT_CMD_SET_ROUTING        7 /* Request to set the routing
                                           * table delivered by Master node*/
#define AXIOM_RT_CMD_INFO               8 /* send a (node_id,if_) pair */
#define AXIOM_RT_CMD_END_INFO           9 /* end of a node routing table */
#define AXIOM_RT_CMD_RT_REPLY          10 /* confirmation of routing table
                                            * reception */
#define AXIOM_CMD_PING                 11 /* Ping request */
#define AXIOM_CMD_PONG                 12 /* Ping reply */
#define AXIOM_CMD_TRACEROUTE           13 /* Traceroute request */
#define AXIOM_CMD_TRACEROUTE_REPLY     14 /* Traceroute reply */
#define AXIOM_CMD_NETPERF              15 /* Network performance request */




#endif /* !AXIOM_NIC_SMALL_COMMANDS_h */
