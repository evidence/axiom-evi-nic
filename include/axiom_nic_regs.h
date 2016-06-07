#ifndef AXIOM_NIC_REGS_H
#define AXIOM_NIC_REGS_H

/*!
 * \file axiom_nic_regs.h
 *
 * \version     v0.5
 * \date        2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations:
 *      - registers offset
 *      - length and element size of registers array
 *      - version macros
 *      - register bitmask and description
 *
 */

/*! \brief axiom-nic compatible id */
#define AXIOMREG_COMPATIBLE                     "evi,axiom-netdev"

/*! \brief length of IFINFO registers array */
#define AXIOMREG_LEN_IFINFO                     8
/*! \brief length of ROUTING registers array */
#define AXIOMREG_LEN_ROUTING                    256
/*! \brief length of RAW QUEUE registers array */
#define AXIOMREG_LEN_RAW_QUEUE                  8
/*! \brief length of RDMA QUEUE registers array */
#define AXIOMREG_LEN_RDMA_QUEUE                 64

/*! \brief size of IFINFO registers array elements */
#define AXIOMREG_SIZE_IFINFO                    1
/*! \brief size of ROUTING registers array elements */
#define AXIOMREG_SIZE_ROUTING                   1
/*! \brief size of RAW QUEUE registers array elements */
#define AXIOMREG_SIZE_RAW_QUEUE                 132
/*! \brief size of RDMA QUEUE registers array elements */
#define AXIOMREG_SIZE_RDMA_QUEUE                12

/*! \brief QEMU board version */
#define AXIOMREG_VER_BRD_QEMU                   0x01

/*! \brief QEMU bitstream version */
#define AXIOMREG_VER_BSR_QEMU                   0x01

/*! \brief QEMU version */
#define AXIOMREG_VER_QEMU ((AXIOMREG_VER_BSR_QEMU << 8) | AXIOMREG_VER_BRD_QEMU)



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


/* Interrupt registers */

/*! \brief ACKIRQ register - 32 bit w/o */
#define AXIOMREG_IO_ACKIRQ                      0x00000060
/*! \brief MKSIRQ register - 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ                      0x00000064
/*! \brief PNDIRQ register - 32 bit r/o */
#define AXIOMREG_IO_PNDIRQ                      0x00000068


/* Routing table registers */

/*! \brief ROUTING_BASE register - 8 bit r/w x 256 = 256 bytes*/
#define AXIOMREG_IO_ROUTING_BASE                0x00000100


/* RAW TX queue registers */

/*! \brief RAW_TX_HEAD register - 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_HEAD                 0x00000300
/*! \brief RAW_TX_TAIL register - 32 bit r/w */
#define AXIOMREG_IO_RAW_TX_TAIL                 0x00000304
/*! \brief RAW_TX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_AVAIL                0x00000308
/*! \brief RAW_TX_BASE register - 1056 bit w/o x 8 = 1056 bytes */
#define AXIOMREG_IO_RAW_TX_BASE                 0x00000400


/* RAW RX queue registers */

/*! \brief RAW_RX_HEAD register - 32 bit r/w */
#define AXIOMREG_IO_RAW_RX_HEAD                 0x00000310
/*! \brief RAW_RX_TAIL register - 32 bit r/o */
#define AXIOMREG_IO_RAW_RX_TAIL                 0x00000314
/*! \brief RAW_RX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_RAW_RX_AVAIL                0x00000318
/*! \brief RAW_RX_BASE register - 1056 r/o x 8 = 1056 bytes */
#define AXIOMREG_IO_RAW_RX_BASE                 0x00000820


/* RDMA TX queue registers */

/*! \brief RDMA_TX_HEAD register - 32 bit r/o */
#define AXIOMREG_IO_RDMA_TX_HEAD                0x00000320
/*! \brief RDMA_TX_TAIL register - 32 bit r/w */
#define AXIOMREG_IO_RDMA_TX_TAIL                0x00000324
/*! \brief RDMA_TX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_RDMA_TX_AVAIL               0x00000328
/*! \brief RDMA_TX_BASE register - 96 bit w/o x 64 = 768 bytes */
#define AXIOMREG_IO_RDMA_TX_BASE                0x00000C40


/* RDMA RX queue registers */

/*! \brief RDMA_RX_HEAD register - 32 bit r/w */
#define AXIOMREG_IO_RDMA_RX_HEAD                0x00000330
/*! \brief RDMA_RX_TAIL register - 32 bit r/o */
#define AXIOMREG_IO_RDMA_RX_TAIL                0x00000334
/*! \brief RDMA_RX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_RDMA_RX_AVAIL               0x00000338
/*! \brief RDMA_RX_BASE register - 96 r/o x 64 = 768 bytes */
#define AXIOMREG_IO_RDMA_RX_BASE                0x00000F40

/*! \brief Registers end */
#define AXIOMREG_IO_SIZE                        0x00001240



/********************* Registers bit description ******************************/


/* Version bit-mask */

#define AXIOMREG_VERSION_BRD_MASK               0x000000FF
#define AXIOMREG_VERSION_BRD_OFF                0
#define AXIOMREG_VERSION_BSR_MASK               0x0000FF00
#define AXIOMREG_VERSION_BSR_OFF                8


/* Interface info status bit */

/*! \brief Interface enabled in TX mode */
#define AXIOMREG_IFINFO_TX                      0x01
/*! \brief Interface enabled in RX mode */
#define AXIOMREG_IFINFO_RX                      0x02
/*! \brief Interface connected */
#define AXIOMREG_IFINFO_CONNECTED               0x04


/* Control bit */

/*! \brief Interface in loopback mode */
#define AXIOMREG_CONTROL_LOOPBACK               0x00000001


/* Interrupt bit */

/*! \brief RAW RX Queue interrupt */
#define AXIOMREG_IRQ_RAW_RX                     0x00000001
/*! \brief RAW TX Queue interrupt */
#define AXIOMREG_IRQ_RAW_TX                     0x00000002
/*! \brief RDMA TX Queue interrupt */
#define AXIOMREG_IRQ_RDMA_TX                    0x00000004
/*! \brief RDMA RX Queue interrupt */
#define AXIOMREG_IRQ_RDMA_RX                    0x00000008
/*! \brief ALL interrupts */
#define AXIOMREG_IRQ_ALL                        0xFFFFFFFF


#endif /* AXIOM_NIC_REGS_H */
