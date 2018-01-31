/*!
 * \file axiom_nic_regs_qemuarm64.h
 *
 * \version     v1.0
 * \date        2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations for QEMU
 * ARM64:
 *      - registers offset
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_REGS_QEMUARM64_H
#define AXIOM_NIC_REGS_QEMUARM64_H

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/************************** Registers Address *********************************/


/* Status registers */

/*! \brief VERSION register - 32 bit r/o */
#define AXIOMREG_IO_VERSION                     0x00000000
/*! \brief STATUS register - 32 bit r/o */
#define AXIOMREG_IO_STATUS                      0x00000004
/*! \brief IFNUMBER register - 32 bit r/o */
#define AXIOMREG_IO_IFNUMBER                    0x00000008
/*! \brief IFINFO_BASE register - 8 bit r/o x 8 = 4 bytes */
#define AXIOMREG_IO_IFINFO_BASE                 0x00000010


/* Control registers */

/*! \brief CONTROL register - 32 bit r/w */
#define AXIOMREG_IO_CONTROL                     0x00000040
/*! \brief NODEID register - 32 bit r/w */
#define AXIOMREG_IO_NODEID                      0x00000044
/*! \brief DMA_START register - 64 bit w/o */
#define AXIOMREG_IO_DMA_START                   0x00000048
/*! \brief DMA_END register - 64 bit w/o */
#define AXIOMREG_IO_DMA_END                     0x00000050



/* Interrupt registers */

/*! \brief MKSIRQ register - 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ                      0x00000060
/*! \brief PNDIRQ register - 32 bit r/w */
#define AXIOMREG_IO_PNDIRQ                      0x00000064


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


/** \} */

#endif /* AXIOM_NIC_REGS_QEMUARM64_H */
