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
#define AXIOMREG_COMPATIBLE             "evi,axiom-netdev"

/*! \brief length of IFINFO registers array */
#define AXIOMREG_LEN_IFINFO             8
/*! \brief length of ROUTING registers array */
#define AXIOMREG_LEN_ROUTING            256
/*! \brief length of SMALL_QUEUE registers array */
#define AXIOMREG_LEN_SMALL_QUEUE        256

/*! \brief size of IFINFO registers array elements */
#define AXIOMREG_SIZE_IFINFO            1
/*! \brief size of ROUTING registers array elements */
#define AXIOMREG_SIZE_ROUTING           1
/*! \brief size of SMALL_QUEUE registers array elements */
#define AXIOMREG_SIZE_SMALL_QUEUE       8

/*! \brief QEMU board version */
#define AXIOMREG_VER_BRD_QEMU           0x01

/*! \brief QEMU bitstream version */
#define AXIOMREG_VER_BSR_QEMU           0x01

/*! \brief QEMU version */
#define AXIOMREG_VER_QEMU ((AXIOMREG_VER_BSR_QEMU << 8) | AXIOMREG_VER_BRD_QEMU)



/************************** Registers Address *********************************/


/* Status registers */

/*! \brief VERSION register - 32 bit r/o */
#define AXIOMREG_IO_VERSION             0x0000
/*! \brief STATUS register - 32 bit r/o */
#define AXIOMREG_IO_STATUS              0x0004
/*! \brief IFNUMBER register - 32 bit r/o */
#define AXIOMREG_IO_IFNUMBER            0x0008
/*! \brief IFINFO_BASE register - 8 bit r/o x 8 = 4 bytes */
#define AXIOMREG_IO_IFINFO_BASE         0x0010


/* Control registers */

/*! \brief CONTROL register - 32 bit r/w */
#define AXIOMREG_IO_CONTROL             0x0040
/*! \brief NODEID register - 32 bit r/w */
#define AXIOMREG_IO_NODEID              0x0044


/* Interrupt registers */

/*! \brief ACKIRQ register - 32 bit w/o */
#define AXIOMREG_IO_ACKIRQ              0x0060
/*! \brief MKSIRQ register - 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ              0x0064
/*! \brief PNDIRQ register - 32 bit r/o */
#define AXIOMREG_IO_PNDIRQ              0x0068


/* Routing table registers */

/*! \brief ROUTING_BASE register - 8 bit r/w x 256 = 256 bytes*/
#define AXIOMREG_IO_ROUTING_BASE        0x0100


/* SMALL TX queue registers */

/*! \brief SMALL_TX_HEAD register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_HEAD       0x0300
/*! \brief SMALL_TX_TAIL register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_TAIL       0x0304
/*! \brief SMALL_TX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_AVAIL      0x0308
/*! \brief SMALL_TX_BASE register - 32 bit w/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_SMALL_TX_BASE       0x0800


/* SMALL RX queue registers */

/*! \brief SMALL_RX_HEAD register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_HEAD       0x0310
/*! \brief SMALL_RX_TAIL register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_TAIL       0x0314
/*! \brief SMALL_RX_AVAIL register - 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_AVAIL      0x0318
/*! \brief SMALL_RX_BASE register - 32 bit r/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_SMALL_RX_BASE       0x1000


/*! \brief Registers end */
#define AXIOMREG_IO_SIZE                0x1800



/********************* Registers bit description ******************************/


/* Version bit-mask */

#define AXIOMREG_VERSION_BRD_MASK       0x000000FF
#define AXIOMREG_VERSION_BRD_OFF        0
#define AXIOMREG_VERSION_BSR_MASK       0x0000FF00
#define AXIOMREG_VERSION_BSR_OFF        8


/* Interface info status bit */

/*! \brief Interface enabled in TX mode */
#define AXIOMREG_IFINFO_TX              0x01
/*! \brief Interface enabled in RX mode */
#define AXIOMREG_IFINFO_RX              0x02
/*! \brief Interface connected */
#define AXIOMREG_IFINFO_CONNECTED       0x04


/* Control bit */

/*! \brief Interface in loopback mode */
#define AXIOMREG_CONTROL_LOOPBACK       0x00000001


/* Interrupt bit */

/*! \brief Interface RX interrupt */
#define AXIOMREG_IRQ_SMALL_RX           0x00000001
/*! \brief Interface TX interrupt */
#define AXIOMREG_IRQ_SMALL_TX           0x00000002
/*! \brief Interface ALL interrupts */
#define AXIOMREG_IRQ_ALL                0xFFFFFFFF


#endif /* AXIOM_NIC_REGS_H */
