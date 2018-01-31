/*!
 * \file axiom_kernel_api.h
 *
 * \version     v1.0
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
#include "axiom_xilinx.h"

typedef union axiom_dev_regs {
    struct {
        axi_reg_t  registers;
        axi_fifo_t fifo_raw_tx;
        axi_fifo_t fifo_raw_rx;
        axi_fifo_t fifo_rdma_tx;
        axi_fifo_t fifo_rdma_rx;
        axi_bram_t long_buf;
        axi_bram_t routing;
        axi_gpio_t debug;
    } axi;

    void __iomem *vregs;
} axiom_dev_regs_t;

/*!
 * \brief Allocate HW api device linked to specified memory mapped registers
 *
 * \param args         Struct with addresses of memory mapped Axiom NIC regs
 *
 * \return A pointer to axiom_dev_t on success, otherwise NULL.
 */
axiom_dev_t *
axiom_hw_dev_alloc(axiom_dev_regs_t *regs);

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

/*!
 * \brief Print AXIOM NIC FPGA debug register
 *
 * \param dev           The axiom device private data pointer
 */
void
axiom_print_fpga_debug(axiom_dev_t *dev);

#endif /* !AXIOM_KERNEL_API_H */
