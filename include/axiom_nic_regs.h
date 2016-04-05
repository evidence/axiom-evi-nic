#ifndef AXIOM_NIC_REGS_H
#define AXIOM_NIC_REGS_H

/*
 * axiom_nic_regs.h
 *
 * Version:     v0.3
 * Last update: 2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations:
 *      - registers offset
 *      - length and element size of registers array
 *      - version macros
 *      - register bitmask and description
 *
 */

/* sysbus compatible id */
#define AXIOMREG_COMPATIBLE             "evi,axiom-netdev"

/* length of registers array */
#define AXIOMREG_LEN_IFINFO             8
#define AXIOMREG_LEN_ROUTING            256
#define AXIOMREG_LEN_SMALL_QUEUE        256

/* size of registers array elements */
#define AXIOMREG_SIZE_IFINFO            1
#define AXIOMREG_SIZE_ROUTING           1
#define AXIOMREG_SIZE_SMALL_QUEUE       8

/* board versions */
#define AXIOMREG_VER_BRD_QEMU           0x01

/* bitstream versions */
#define AXIOMREG_VER_BSR_QEMU           0x01

/* versions */
#define AXIOMREG_VER_QEMU ((AXIOMREG_VER_BSR_QEMU << 8) | AXIOMREG_VER_BRD_QEMU)


/************************** Registers Address *********************************/

/* Status registers */
/* 32 bit r/o */
#define AXIOMREG_IO_VERSION             0x0000
/* 32 bit r/o */
#define AXIOMREG_IO_STATUS              0x0004
/* 32 bit r/o */
#define AXIOMREG_IO_IFNUMBER            0x0008
/* 8 bit r/o x 8 = 4 bytes */
#define AXIOMREG_IO_IFINFO_BASE         0x0010

/* Control registers */
/* 32 bit r/w */
#define AXIOMREG_IO_CONTROL             0x0040
/* 32 bit r/w */
#define AXIOMREG_IO_NODEID              0x0044

/* Interrupt registers */
/* 32 bit w/o */
#define AXIOMREG_IO_ACKIRQ              0x0060
/* 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ              0x0064
/* 32 bit r/o */
#define AXIOMREG_IO_PNDIRQ              0x0068

/* Routing table registers */
/* 8 bit r/w x 256 = 256 bytes*/
#define AXIOMREG_IO_ROUTING_BASE        0x0100

/* SMALL TX queue registers */
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_HEAD       0x0300
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_TAIL       0x0304
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_TX_AVAIL      0x0308
/* 32 bit w/o */
#define AXIOMREG_IO_SMALL_TX_PUSH       0x030C
/* 32 bit w/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_SMALL_TX_BASE       0x0800

/* SMALL RX queue registers */
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_HEAD       0x0310
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_TAIL       0x0314
/* 32 bit r/o */
#define AXIOMREG_IO_SMALL_RX_AVAIL      0x0318
/* 32 bit w/o */
#define AXIOMREG_IO_SMALL_RX_POP        0x031C
/* 32 bit r/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_SMALL_RX_BASE       0x1000


/* Registers end */
#define AXIOMREG_IO_SIZE                0x1800



/********************* Registers bit description ******************************/

/* Version bit-mask */
#define AXIOMREG_VERSION_BRD_MASK       0x000000FF
#define AXIOMREG_VERSION_BRD_OFF        0
#define AXIOMREG_VERSION_BSR_MASK       0x0000FF00
#define AXIOMREG_VERSION_BSR_OFF        8

/* Interface info status bit */
#define AXIOMREG_IFINFO_TX              0x01
#define AXIOMREG_IFINFO_RX              0x02
#define AXIOMREG_IFINFO_CONNECTED       0x04

/* Control bit */
#define AXIOMREG_CONTROL_LOOPBACK       0x00000001

/* Interrupt bit */
#define AXIOMREG_IRQ_SMALL_RX           0x00000001
#define AXIOMREG_IRQ_SMALL_TX           0x00000002
#define AXIOMREG_IRQ_ALL                0xFFFFFFFF


#endif /* AXIOM_NIC_REGS_H */
