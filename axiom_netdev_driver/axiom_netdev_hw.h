#ifndef AXIOM_NETDEV_REGS_H
#define AXIOM_NETDEV_REGS_H

/* sysbus compatible id */
#define AXIOMREG_COMPATIBLE             "evi,axiom-netdev"

/* length of registers array */
#define AXIOMREG_LEN_IFINFO             4
#define AXIOMREG_LEN_ROUTING            256
#define AXIOMREG_LEN_RAW_QUEUE          256

/* board versions */
#define AXIOMREG_VER_BRD_QEMU           0x01

/* bitstream versions */
#define AXIOMREG_VER_BSR_QEMU           0x01


/************************** Registers Address *********************************/

/* Status registers */
/* 32 bit r/o */
#define AXIOMREG_IO_VERSION             0x0000
/* 32 bit r/o */
#define AXIOMREG_IO_STATUS              0x0004
/* 32 bit r/o */
#define AXIOMREG_IO_IFNUMBER            0x0008
/* 32 bit r/o x 4 = 16 bytes */
#define AXIOMREG_IO_IFINFO_BASE         0x000C

/* Control registers */
/* 32 bit w/o */
#define AXIOMREG_IO_CONTROL             0x0040
/* 32 bit r/w */
#define AXIOMREG_IO_NODEID              0x0044

/* Interrupt registers */
/* 32 bit w/o */
#define AXIOMREG_IO_ACKIRQ              0x0060
/* 32 bit r/w */
#define AXIOMREG_IO_SETIRQ              0x0064
/* 32 bit r/o */
#define AXIOMREG_IO_PNDIRQ              0x0068

/* Routing table registers */
/* 8 bit r/w x 256 = 256 bytes*/
#define AXIOMREG_IO_ROUTING_BASE        0x0080

/* RAW TX queue registers */
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_HEAD         0x0200
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_TAIL         0x0204
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_TX_INFO         0x0208
/* 32 bit w/o */
#define AXIOMREG_IO_RAW_TX_START        0x020C
/* 32 bit w/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_RAW_TX_BASE         0x0210

/* RAW RX queue registers */
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_RX_HEAD         0x0C00
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_RX_TAIL         0x0C04
/* 32 bit r/o */
#define AXIOMREG_IO_RAW_RX_INFO         0x0C08
/* 32 bit w/o */
#define AXIOMREG_IO_RAW_RX_START        0x0C0C
/* 32 bit r/o x 2 x 256 = 2048 bytes */
#define AXIOMREG_IO_RAW_RX_BASE         0x0C10


/* Registers end */
#define AXIOMREG_IO_SIZE                0x1410



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

/* Interrupt bit */
#define AXIOMREG_IRQ_GENERIC            0x00000001
#define AXIOMREG_IRQ_RAW_RX             0x00000002
#define AXIOMREG_IRQ_RAW_TX             0x00000004
#define AXIOMREG_IRQ_ALL                0xFFFFFFFF

/* Queue status bit */
#define AXIOMREG_RAW_STATUS_EMPTY       0x01
#define AXIOMREG_RAW_STATUS_FULL        0x02


#endif /* AXIOM_NETDEV_REGS_H */
