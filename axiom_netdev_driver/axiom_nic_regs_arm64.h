/*!
 * \file axiom_nic_regs_arm64.h
 *
 * \version     v1.2
 * \date        2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations for the
 * FORTH bitstream:
 *      - registers offset
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_REGS_ARM64_H
#define AXIOM_NIC_REGS_ARM64_H

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/************************** Registers Address *********************************/


/* Status registers */

/*! \brief VERSION register - 32 bit r/o */
#define AXIOMREG_IO_VERSION                     \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_VERSION_V_DATA
/*! \brief STATUS register - 32 bit r/o - [NOT IMPLEMENTED] */
#define AXIOMREG_IO_STATUS                      (-1)
/*! \brief IFNUMBER register - 32 bit r/o */
#define AXIOMREG_IO_IFNUMBER                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_IFNUMBER_V_DATA
/*! \brief IFINFO 1 register - 32 bit r/o */
#define AXIOMREG_IO_IFINFO_1                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE1_V_DATA
/*! \brief IFINFO 2 register - 32 bit r/o */
#define AXIOMREG_IO_IFINFO_2                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE2_V_DATA
/*! \brief IFINFO 3 register - 32 bit r/o */
#define AXIOMREG_IO_IFINFO_3                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE3_V_DATA
/*! \brief IFINFO 4 register - 32 bit r/o */
#define AXIOMREG_IO_IFINFO_4                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_IFINFOBASE4_V_DATA


/* Control registers */

/*! \brief CONTROL register - 32 bit r/w */
#define AXIOMREG_IO_CONTROL                     \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_CONTROL_V_DATA
/*! \brief NODEID register - 32 bit r/w */
#define AXIOMREG_IO_NODEID                      \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_NODEID_V_DATA
/*! \brief DMA_START register - 64 bit w/o */
#define AXIOMREG_IO_DMA_START                   \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_DMASTART_V_DATA
/*! \brief DMA_END register - 64 bit w/o */
#define AXIOMREG_IO_DMA_END                     \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_DMAEND_V_DATA
/*! \brief AXI Signal for memory attributes r/w transaction - 32 bit w/o */
#define AXIOMREG_IO_AXCACHE                    \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_AXCACHE_V_DATA
/*! \brief AXI Signal for access permissions r/w transaction - 32 bit w/o */
#define AXIOMREG_IO_AXPROT                      \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_AXPROT_V_DATA

/*! \brief Enable cache coherency AXI transaction */
#define AXIOMREG_AXCACHE_ENABLE                 0x0000000F
/*! \brief Enable non-secure (Linux) AXI transaction */
#define AXIOMREG_AXPROT_ENABLE                  0x00000002

/* Interrupt registers */

/*! \brief MKSIRQ register - 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ                      \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_MSKIRQ_V_DATA
/*! \brief PNDIRQ register - 32 bit r/w */
#define AXIOMREG_IO_PNDIRQ                      \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_PNDIRQ_V_DATA
/*! \brief CLRIRQ register - 32 bit r/w */
#define AXIOMREG_IO_CLRIRQ                      \
        XREGISTER_CONTROLLER_REGISTERS_ADDR_CLRIRQ_V_DATA


#if 0 /* following registers are not implemented */
/* Routing table registers */

/*! \brief ROUTING_BASE register - 8 bit r/w x 256 = 256 bytes*/
#define AXIOMREG_IO_ROUTING_BASE                0x00000100


/* RAW TX queue registers */

/*! \brief RAW_TX_STATUS register - 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_STATUS               0x00000300
/*! \brief RAW_TX_DESC register - 1056 bit w/o */
#define AXIOMREG_IO_RAW_TX_DESC                 0x00000400


/* RAW RX queue registers */

/*! \brief RAW_RX_STATUS register - 32 bit r/w */
#define AXIOMREG_IO_RAW_RX_STATUS               0x00000310
/*! \brief RAW_RX_DESC register - 1056 bit r/o */
#define AXIOMREG_IO_RAW_RX_DESC                 0x00000500


/* RDMA TX queue registers */

/*! \brief RDMA_TX_STATUS register - 32 bit r/o */
#define AXIOMREG_IO_RDMA_TX_STATUS              0x00000320
/*! \brief RDMA_TX_DESC register - 96 bit w/o */
#define AXIOMREG_IO_RDMA_TX_DESC                0x00000600


/* RDMA RX queue registers */

/*! \brief RDMA_RX_STATUS register - 32 bit r/w */
#define AXIOMREG_IO_RDMA_RX_STATUS              0x00000330
/*! \brief RDMA_RX_DESC register - 96 bit r/o */
#define AXIOMREG_IO_RDMA_RX_DESC                0x00000610

/* LONG buffer registers */
/*! \brief LONG_BUF_BASE register - 64 bit r/w x 32 = 256 bytes */
#define AXIOMREG_IO_LONG_BUF_BASE               0x00000620

/*! \brief Registers end */
#define AXIOMREG_IO_SIZE                        0x00000720
#endif

/** \} */

#endif /* AXIOM_NIC_REGS_ARM64_H */
