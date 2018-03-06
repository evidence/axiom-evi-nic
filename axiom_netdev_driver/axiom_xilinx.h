/*!
 * \file axiom_xilinx.h
 *
 * \version     v1.1
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

typedef struct axi_reg {
    void __iomem *base_addr;
} axi_reg_t;

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

/******************************* xregisters ***********************************/

/************************** Constant Definitions *****************************/

// registers
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : Data signal of DMAend_V
//        bit 31~0 - DMAend_V[31:0] (Read/Write)
// 0x14 : Data signal of DMAend_V
//        bit 31~0 - DMAend_V[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of DMAstart_V
//        bit 31~0 - DMAstart_V[31:0] (Read/Write)
// 0x20 : Data signal of DMAstart_V
//        bit 31~0 - DMAstart_V[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of NodeId_V
//        bit 7~0 - NodeId_V[7:0] (Read/Write)
//        others  - reserved
// 0x2c : reserved
// 0x30 : Data signal of MSKirq_V
//        bit 3~0 - MSKirq_V[3:0] (Read/Write)
//        others  - reserved
// 0x34 : reserved
// 0x38 : Data signal of PNDirq_V
//        bit 3~0 - PNDirq_V[3:0] (Read)
//        others  - reserved
// 0x3c : Control signal of PNDirq_V
//        bit 0  - PNDirq_V_ap_vld (Read/COR)
//        others - reserved
// 0x40 : Data signal of CLRirq_V
//        bit 3~0 - CLRirq_V[3:0] (Read/Write)
//        others  - reserved
// 0x44 : reserved
// 0x48 : Data signal of IFinfoBase1_V
//        bit 2~0 - IFinfoBase1_V[2:0] (Read)
//        others  - reserved
// 0x4c : reserved
// 0x50 : Data signal of IFinfoBase2_V
//        bit 2~0 - IFinfoBase2_V[2:0] (Read)
//        others  - reserved
// 0x54 : reserved
// 0x58 : Data signal of IFinfoBase3_V
//        bit 2~0 - IFinfoBase3_V[2:0] (Read)
//        others  - reserved
// 0x5c : reserved
// 0x60 : Data signal of IFinfoBase4_V
//        bit 2~0 - IFinfoBase4_V[2:0] (Read)
//        others  - reserved
// 0x64 : reserved
// 0x68 : Data signal of Version_V
//        bit 15~0 - Version_V[15:0] (Read)
//        others   - reserved
// 0x6c : reserved
// 0x70 : Data signal of IFnumber_V
//        bit 2~0 - IFnumber_V[2:0] (Read)
//        others  - reserved
// 0x74 : reserved
// 0x78 : Data signal of Control_V
//        bit 1~0 - Control_V[1:0] (Read/Write)
//        others - reserved
// 0x7c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XREGISTER_CONTROLLER_REGISTERS_ADDR_DMAEND_V_DATA      0x10
#define XREGISTER_CONTROLLER_REGISTERS_BITS_DMAEND_V_DATA      64
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_DMASTART_V_DATA    0x1c
#define XREGISTER_CONTROLLER_REGISTERS_BITS_DMASTART_V_DATA    64
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_NODEID_V_DATA      0x28
#define XREGISTER_CONTROLLER_REGISTERS_BITS_NODEID_V_DATA      8
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_MSKIRQ_V_DATA      0x30
#define XREGISTER_CONTROLLER_REGISTERS_BITS_MSKIRQ_V_DATA      4
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_PNDIRQ_V_DATA      0x38
#define XREGISTER_CONTROLLER_REGISTERS_BITS_PNDIRQ_V_DATA      4
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_CLRIRQ_V_DATA      0x40
#define XREGISTER_CONTROLLER_REGISTERS_BITS_CLRIRQ_V_DATA      4
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE1_V_DATA 0x48
#define XREGISTER_CONTROLLER_REGISTERS_BITS_IFINFOBASE1_V_DATA 3
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE2_V_DATA 0x50
#define XREGISTER_CONTROLLER_REGISTERS_BITS_IFINFOBASE2_V_DATA 3
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE3_V_DATA 0x58
#define XREGISTER_CONTROLLER_REGISTERS_BITS_IFINFOBASE3_V_DATA 3
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE4_V_DATA 0x60
#define XREGISTER_CONTROLLER_REGISTERS_BITS_IFINFOBASE4_V_DATA 3
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_VERSION_V_DATA     0x68
#define XREGISTER_CONTROLLER_REGISTERS_BITS_VERSION_V_DATA     16
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_IFNUMBER_V_DATA    0x70
#define XREGISTER_CONTROLLER_REGISTERS_BITS_IFNUMBER_V_DATA    3
#define XREGISTER_CONTROLLER_REGISTERS_ADDR_CONTROL_V_DATA     0x78
#define XREGISTER_CONTROLLER_REGISTERS_BITS_CONTROL_V_DATA     2

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

static inline uint32_t
axi_reg_read32(axi_reg_t *reg, int offset)
{
    return ioread32(reg->base_addr + offset);
}

static inline void
axi_reg_write32(axi_reg_t *reg, int offset, uint32_t value)
{
    iowrite32(value, reg->base_addr + offset);
}

static inline uint64_t
axi_reg_read64(axi_reg_t *reg, int offset)
{
    uint64_t value;

    value = ioread32(reg->base_addr + offset);
    value += ((uint64_t)ioread32(reg->base_addr + offset + 4)) << 32;

    return value;
}

static inline void
axi_reg_write64(axi_reg_t *reg, int offset, uint64_t value)
{
    iowrite32(value, reg->base_addr + offset);
    iowrite32((value >> 32), reg->base_addr + offset + 4);
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
