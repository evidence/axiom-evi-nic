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
#define AXIOMNET_IO_TXD_BASE            20
/* 32 bit w/o */
#define AXIOMNET_IO_TX_START            24
/* 32 bit w/o */
#define AXIOMNET_IO_RXD_BASE            28
/* 32 bit w/o */
#define AXIOMNET_IO_RX_START            32
/* 32 bit w/o */
#define AXIOMNET_IO_BUFSIZE             36

#define AXIOMNET_IO_SIZE                40

/* Interrupt identifier */
#define AXIOMNET_IRQ_GENERIC            0x00000001
#define AXIOMNET_IRQ_RX                 0x00000002
#define AXIOMNET_IRQ_ALL                0xFFFFFFFF

struct axiom_netdev_bufdesc {
    uint64_t buf_addr;
    uint32_t length;
    uint32_t flags;
};

/* Buffer flags */
#define AXIOMNET_BUFDESC_EMPTY          0x00000001
#define AXIOMNET_BUFDESC_LAST           0x00000002


#endif /* AXIOM_NETDEV_REGS_H */
