/*!
 * \file axiom_kernel_api.h
 *
 * \version     v0.14
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
        axi_gpio_t version;
        axi_gpio_t ifnumber;
        axi_gpio_t ifinfobase0;
        axi_gpio_t ifinfobase1;
        axi_gpio_t nodeid;
        axi_gpio_t mskirq;
        axi_gpio_t pndirq;
        axi_gpio_t dma_start;
        axi_gpio_t dma_end;
        axi_gpio_t rtctrl;
        axi_gpio_t aur_ctrl0;
        axi_gpio_t aur_ctrl1;
        axi_gpio_t aur_status0;
        axi_gpio_t aur_status1;
        axi_fifo_t fifo_raw_tx;
        axi_fifo_t fifo_raw_rx;
        axi_fifo_t fifo_rdma_tx;
        axi_fifo_t fifo_rdma_rx;
        axi_bram_t long_buf;
        axi_bram_t rt_phy;
        axi_bram_t rt_rx;
        axi_bram_t rt_tx_dma;
        axi_bram_t rt_tx_raw;
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

#endif /* !AXIOM_KERNEL_API_H */
