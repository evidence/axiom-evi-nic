#ifndef AXIOM_NETDEV_REGS_H
#define AXIOM_NETDEV_REGS_H

/* sysbus compatible id */
#define AXIOMNET_COMPATIBLE             "evi,axiom-netdev"

/* Register Address */
/* 32 bit r/o */
#define AXIOMNET_IO_FEATURES            0
/* 32 bit w/o */
#define AXIOMNET_IO_ACKIRQ              4
/* 32 bit r/w */
#define AXIOMNET_IO_SETIRQ              8
/* 32 bit r/o */
#define AXIOMNET_IO_PNDIRQ              12
/* 32 bit r/w */
#define AXIOMNET_IO_NODEID              16
/* 32 bit w/o */
#define AXIOMNET_IO_RAW_TX_BASE         20
/* 32 bit w/o */
#define AXIOMNET_IO_RAW_TX_LEN          24
/* 32 bit w/o */
#define AXIOMNET_IO_RAW_TX_HEAD         28
/* 32 bit w/o */
#define AXIOMNET_IO_RAW_TX_TAIL         32

#define AXIOMNET_IO_SIZE                36

/* Interrupt identifier */
#define AXIOMNET_IRQ_GENERIC            0x00000001
#define AXIOMNET_IRQ_RX                 0x00000002
#define AXIOMNET_IRQ_ALL                0xFFFFFFFF

/* Default parameters */
#define AXIOMNET_DEF_RING_LEN           256


#endif /* AXIOM_NETDEV_REGS_H */
