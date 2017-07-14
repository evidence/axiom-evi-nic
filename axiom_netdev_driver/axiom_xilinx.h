/*!
 * \file axiom_xilinx.h
 *
 * \version     v0.13
 * \date        2017-06-30
 *
 * This file contains the AXIOM XILINX macros generated from Vivado.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_XILINX_H
#define AXIOM_XILINX_H

typedef struct axi_fifo {
    void __iomem *base_addr;
    void __iomem *axi4_base_addr;
    int axi4;
} axi_fifo_t;

typedef struct axi_gpio {
    void __iomem *base_addr;
    int dual;
} axi_gpio_t;

typedef struct axi_bram {
    void __iomem *base_addr;
} axi_bram_t;

/******************************** xllfifo ************************************/

/************************** Constant Definitions *****************************/

/* Register offset definitions. Unless otherwise noted, register access is
 * 32 bit.
 */

/** @name Registers
 *  @{
 */
#define XLLF_ISR_OFFSET  0x00000000  /**< Interrupt Status */
#define XLLF_IER_OFFSET  0x00000004  /**< Interrupt Enable */

#define XLLF_TDFR_OFFSET 0x00000008  /**< Transmit Reset */
#define XLLF_TDFV_OFFSET 0x0000000c  /**< Transmit Vacancy */
#define XLLF_TDFD_OFFSET 0x00000010  /**< Transmit Data */
#define XLLF_AXI4_TDFD_OFFSET   0x00000000  /**< Axi4 Transmit Data */
#define XLLF_TLF_OFFSET  0x00000014  /**< Transmit Length */

#define XLLF_RDFR_OFFSET 0x00000018  /**< Receive Reset */
#define XLLF_RDFO_OFFSET 0x0000001c  /**< Receive Occupancy */
#define XLLF_RDFD_OFFSET 0x00000020  /**< Receive Data */
#define XLLF_AXI4_RDFD_OFFSET 	0x00001000  /**< Axi4 Receive Data */
#define XLLF_RLF_OFFSET  0x00000024  /**< Receive Length */
#define XLLF_LLR_OFFSET  0x00000028  /**< Local Link Reset */
#define XLLF_TDR_OFFSET  0x0000002C  /**< Transmit Destination  */
#define XLLF_RDR_OFFSET  0x00000030  /**< Receive Destination  */

/*@}*/

/* Register masks. The following constants define bit locations of various
 * control bits in the registers. Constants are not defined for those registers
 * that have a single bit field representing all 32 bits. For further
 * information on the meaning of the various bit masks, refer to the HW spec.
 */

/** @name Interrupt bits
 *  These bits are associated with the XLLF_IER_OFFSET and XLLF_ISR_OFFSET
 *  registers.
 * @{
 */
#define XLLF_INT_RPURE_MASK       0x80000000 /**< Receive under-read */
#define XLLF_INT_RPORE_MASK       0x40000000 /**< Receive over-read */
#define XLLF_INT_RPUE_MASK        0x20000000 /**< Receive underrun (empty) */
#define XLLF_INT_TPOE_MASK        0x10000000 /**< Transmit overrun */
#define XLLF_INT_TC_MASK          0x08000000 /**< Transmit complete */
#define XLLF_INT_RC_MASK          0x04000000 /**< Receive complete */
#define XLLF_INT_TSE_MASK         0x02000000 /**< Transmit length mismatch */
#define XLLF_INT_TRC_MASK         0x01000000 /**< Transmit reset complete */
#define XLLF_INT_RRC_MASK         0x00800000 /**< Receive reset complete */
#define XLLF_INT_TFPF_MASK        0x00400000 /**< Tx FIFO Programmable Full,
						* AXI FIFO MM2S Only */
#define XLLF_INT_TFPE_MASK        0x00200000 /**< Tx FIFO Programmable Empty
						* AXI FIFO MM2S Only */
#define XLLF_INT_RFPF_MASK        0x00100000 /**< Rx FIFO Programmable Full
						* AXI FIFO MM2S Only */
#define XLLF_INT_RFPE_MASK        0x00080000 /**< Rx FIFO Programmable Empty
						* AXI FIFO MM2S Only */
#define XLLF_INT_ALL_MASK         0xfff80000 /**< All the ints */
#define XLLF_INT_ERROR_MASK       0xf2000000 /**< Error status ints */
#define XLLF_INT_RXERROR_MASK     0xe0000000 /**< Receive Error status ints */
#define XLLF_INT_TXERROR_MASK     0x12000000 /**< Transmit Error status ints */
/*@}*/

/** @name Reset register values
 *  These bits are associated with the XLLF_TDFR_OFFSET and XLLF_RDFR_OFFSET
 *  reset registers.
 * @{
 */
#define XLLF_RDFR_RESET_MASK        0x000000a5 /**< receive reset value */
#define XLLF_TDFR_RESET_MASK        0x000000a5 /**< Transmit reset value */
#define XLLF_LLR_RESET_MASK         0x000000a5 /**< Local Link reset value */
/*@}*/

/********************************* xgpio *************************************/

/************************** Constant Definitions *****************************/

/** @name Registers
 *
 * Register offsets for this device.
 * @{
 */
#define XGPIO_DATA_OFFSET	0x0   /**< Data register for 1st channel */
#define XGPIO_TRI_OFFSET	0x4   /**< I/O direction reg for 1st channel */
#define XGPIO_DATA2_OFFSET	0x8   /**< Data register for 2nd channel */
#define XGPIO_TRI2_OFFSET	0xC   /**< I/O direction reg for 2nd channel */

#define XGPIO_GIE_OFFSET	0x11C /**< Glogal interrupt enable register */
#define XGPIO_ISR_OFFSET	0x120 /**< Interrupt status register */
#define XGPIO_IER_OFFSET	0x128 /**< Interrupt enable register */

/* @} */

/* The following constant describes the offset of each channels data and
 * tristate register from the base address.
 */
#define XGPIO_CHAN_OFFSET  8

/** @name Interrupt Status and Enable Register bitmaps and masks
 *
 * Bit definitions for the interrupt status register and interrupt enable
 * registers.
 * @{
 */
#define XGPIO_IR_MASK		0x3 /**< Mask of all bits */
#define XGPIO_IR_CH1_MASK	0x1 /**< Mask for the 1st channel */
#define XGPIO_IR_CH2_MASK	0x2 /**< Mask for the 2nd channel */
/*@}*/


/** @name Global Interrupt Enable Register bitmaps and masks
 *
 * Bit definitions for the Global Interrupt  Enable register
 * @{
 */
#define XGPIO_GIE_GINTR_ENABLE_MASK	0x80000000
/*@}*/

static inline uint32_t
axi_gpio_read32(axi_gpio_t *gpio)
{
    return ioread32(gpio->base_addr + XGPIO_DATA_OFFSET);
}

static inline void
axi_gpio_write32(axi_gpio_t *gpio, uint32_t value)
{
    iowrite32(value, gpio->base_addr + XGPIO_DATA_OFFSET);
}

static inline void
axi_gpio_write64(axi_gpio_t *gpio, uint64_t value)
{
    iowrite32(value, gpio->base_addr + XGPIO_DATA_OFFSET);
    iowrite32((value >> 32), gpio->base_addr + XGPIO_DATA2_OFFSET);
}

static inline void
axi_fifo_reset(axi_fifo_t *fifo)
{
    DPRINTF("befo)e reset - status: 0x%x",
            ioread32(fifo->base_addr + XLLF_ISR_OFFSET));

    iowrite32(XLLF_INT_ALL_MASK, fifo->base_addr + XLLF_ISR_OFFSET);

    iowrite32(XLLF_TDFR_RESET_MASK, fifo->base_addr + XLLF_TDFR_OFFSET);
    iowrite32(XLLF_RDFR_RESET_MASK, fifo->base_addr + XLLF_RDFR_OFFSET);
    iowrite32(XLLF_LLR_RESET_MASK, fifo->base_addr + XLLF_LLR_OFFSET);

    DPRINTF("after reset - status: 0x%x",
            ioread32(fifo->base_addr + XLLF_ISR_OFFSET));

    iowrite32(XLLF_INT_ALL_MASK, fifo->base_addr + XLLF_ISR_OFFSET);
}

static inline uint32_t
axi_fifo_tx_vacancy(axi_fifo_t *fifo)
{
    return ioread32(fifo->base_addr + XLLF_TDFV_OFFSET);
}

static inline void
axi_fifo_tx_setlen(axi_fifo_t *fifo, uint32_t bytes)
{
    iowrite32(bytes, fifo->base_addr + XLLF_TLF_OFFSET);
}

static inline uint32_t
axi_fifo_rx_occupancy(axi_fifo_t *fifo)
{
    return ioread32(fifo->base_addr + XLLF_RDFO_OFFSET);
}

static inline uint32_t
axi_fifo_rx_getlen(axi_fifo_t *fifo)
{
    return ioread32(fifo->base_addr + XLLF_RLF_OFFSET);
}

static inline void
axi_fifo_write64(axi_fifo_t *fifo, uint64_t value)
{
    writeq(value, fifo->axi4_base_addr + XLLF_AXI4_TDFD_OFFSET);
}

static inline uint64_t
axi_fifo_read64(axi_fifo_t *fifo)
{
    return readq(fifo->axi4_base_addr + XLLF_AXI4_RDFD_OFFSET);
}

static inline uint32_t
axi_bram_read32(axi_bram_t *bram, uint64_t offset)
{
    return ioread32(bram->base_addr + offset);
}

static inline void
axi_bram_write32(axi_bram_t *bram, uint64_t offset, uint32_t value)
{
    iowrite32(value, bram->base_addr + offset);
}

static inline void
axi_bram_write64(axi_bram_t *bram, uint64_t offset, uint64_t value)
{
    writeq(value, bram->base_addr + offset);
}
#endif /* AXIOM_XILINX_H */
