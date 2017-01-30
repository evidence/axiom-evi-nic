/*!
 * \file axiom_switch.h
 *
 * \version     v0.11
 * \date        2016-05-03
 *
 * This file contains some macro for Axiom Switch application
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_SWITCH_h
#define AXIOM_SWITCH_h

#include "dprintf.h"
#include "axiom_nic_types.h"
#include "axiom_nic_limits.h"
#include "axiom_sim_topology.h"
#include "axiom_utility.h"

/*! \brief maximum port supported in the axiom switch */
#define AXSW_PORT_MAX           16
/*! \brief first TCP port to listen QEMU VMs */
#define AXSW_PORT_START         33300

/*! \brief default buffer size */
#define AXSW_BUF_SIZE           1024
/*! \brief pollfd array size */
#define AXSW_FDS_SIZE           AXSW_PORT_MAX*2

#endif /* AXIOM_SWITCH_LOGIC_h */
