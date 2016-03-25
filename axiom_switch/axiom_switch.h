#ifndef AXIOM_SWITCH_h
#define AXIOM_SWITCH_h

#define PDEBUG
#include "dprintf.h"
#include "axiom_topology.h"

#define AXSW_PORT_MAX           16      /* max port supported */
#define AXSW_PORT_START         33300   /* first port to listen */

#define AXSW_BUF_SIZE           1024
#define AXSW_FDS_SIZE           AXSW_PORT_MAX*2

#ifdef EXAMPLE1
#define AXTP_EXAMPLE1   EXAMPLE1
#endif
#ifdef EXAMPLE2
#define AXTP_EXAMPLE2   EXAMPLE2
#endif
#ifdef EXAMPLE3
#define AXTP_EXAMPLE3   EXAMPLE3
#endif
#ifdef EXAMPLE4
#define AXTP_EXAMPLE4   EXAMPLE4
#endif
#ifdef EXAMPLE5
#define AXTP_EXAMPLE5   EXAMPLE5
#endif

#define AXTP_NUM_NODES      AXIOM_NUM_NODES
/* Max number of connected node interfaces */
#define AXTP_NUM_INTERFACES AXIOM_NUM_INTERFACES
#define AXTP_NULL_NODE      AXIOM_NULL_NODE

#endif /* AXIOM_SWITCH_LOGIC_h */
