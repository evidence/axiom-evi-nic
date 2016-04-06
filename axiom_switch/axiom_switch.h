#ifndef AXIOM_SWITCH_h
#define AXIOM_SWITCH_h

#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_sim_topology.h"

#define AXSW_PORT_MAX           16      /* max port supported */
#define AXSW_PORT_START         33300   /* first port to listen */

#define AXSW_BUF_SIZE           1024
#define AXSW_FDS_SIZE           AXSW_PORT_MAX*2

/* Max number of connected node interfaces */
#define AXTP_NUM_INTERFACES AXIOM_MAX_INTERFACES
#define AXTP_NULL_NODE      AXIOM_NULL_NODE

#endif /* AXIOM_SWITCH_LOGIC_h */
