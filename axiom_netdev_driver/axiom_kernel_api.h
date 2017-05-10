/*!
 * \file axiom_kernel_api.h
 *
 * \version     v0.12
 * \date        2016-05-03
 *
 * This file contains the Axiom NIC hardware API prototypes.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_KERNEL_API_H
#define AXIOM_KERNEL_API_H

#include "dprintf.h"
#include "axiom_nic_api_hw.h"

/*!
 * \brief Allocate HW api device linked to specified memory mapped registers
 *
 * \param vregs         Address of memory mapped Axiom NIC registers
 *
 * \return A pointer to axiom_dev_t on success, otherwise NULL.
 */
axiom_dev_t *
axiom_hw_dev_alloc(void *vregs);

/*!
 * \brief Free HW API device
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_hw_dev_free(axiom_dev_t *dev);



/*************************** debug functions **********************************/

/*!
 * \brief Print AXIOM NIC status register
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_print_status_reg(axiom_dev_t *dev);

/*!
 * \brief Print AXIOM NIC control register
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_print_control_reg(axiom_dev_t *dev);

/*!
 * \brief Print AXIOM NIC routing registers
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_print_routing_reg(axiom_dev_t *dev);

/*!
 * \brief Print AXIOM NIC queue registers
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_print_queue_reg(axiom_dev_t *dev);

#endif /* !AXIOM_KERNEL_API_H */
