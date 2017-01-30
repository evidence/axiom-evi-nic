/*!
 * \file axiom_nic_regs.h
 *
 * \version     v0.11
 * \date        2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations:
 *      - registers offset
 *      - length and element size of registers array
 *      - version macros
 *      - register bitmask and description
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_REGS_H
#define AXIOM_NIC_REGS_H

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/*! \brief axiom-nic compatible id */
#define AXIOMREG_COMPATIBLE                     "evi,axiom-netdev"

/*! \brief length of IFINFO registers array */
#define AXIOMREG_LEN_IFINFO                     8
/*! \brief length of ROUTING registers array */
#define AXIOMREG_LEN_ROUTING                    256
/*! \brief length of LONG_BUF registers array */
#define AXIOMREG_LEN_LONG_BUF                   32
/*! \brief length of RAW QUEUE registers array */
//#define AXIOMREG_LEN_RAW_QUEUE                  1
/*! \brief length of RDMA QUEUE registers array */
//#define AXIOMREG_LEN_RDMA_QUEUE                 1

/*! \brief size of IFINFO registers array elements */
#define AXIOMREG_SIZE_IFINFO                    1
/*! \brief size of ROUTING registers array elements */
#define AXIOMREG_SIZE_ROUTING                   1
/*! \brief size of LONG_BUF registers array elements */
#define AXIOMREG_SIZE_LONG_BUF                  8
/*! \brief size of RAW QUEUE registers array elements */
#define AXIOMREG_SIZE_RAW_QUEUE   \
    (AXIOM_RAW_HEADER_SIZE + AXIOM_RAW_PAYLOAD_MAX_SIZE)
/*! \brief size of RDMA QUEUE registers array elements */
#define AXIOMREG_SIZE_RDMA_QUEUE                AXIOM_RDMA_HEADER_SIZE

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
/*! \brief DMA_START register - 64 bit w/o */
#define AXIOMREG_IO_DMA_START                   0x00000048
/*! \brief DMA_END register - 64 bit w/o */
#define AXIOMREG_IO_DMA_END                     0x00000050



/* Interrupt registers */

/*! \brief MKSIRQ register - 32 bit r/w */
#define AXIOMREG_IO_MSKIRQ                      0x00000060
/*! \brief PNDIRQ register - 32 bit r/w */
#define AXIOMREG_IO_PNDIRQ                      0x00000064
/*! \brief AVLIRQ register - 32 bit r/w */
#define AXIOMREG_IO_AVLIRQ                      0x00000068


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



/********************* Registers bit description ******************************/

/*!
 * \brief AXIOM register: VERSION bit field
 */
typedef union axiomreg_version {
    uint32_t raw;
    struct {
        uint32_t reserved : 16;     /*!< \brief Reserved field */
        uint32_t bitstream : 8;     /*!< \brief Bitstream version */
        uint32_t board : 8;         /*!< \brief Board version */
    } field;
} axiomreg_version_t;
#define AXIOMREG_VERSION_BRD_MASK               0x000000FF
#define AXIOMREG_VERSION_BRD_OFF                0
#define AXIOMREG_VERSION_BSR_MASK               0x0000FF00
#define AXIOMREG_VERSION_BSR_OFF                8


/*!
 * \brief AXIOM register: IFINFO bit field
 */
typedef union axiomreg_ifinfo {
    uint32_t raw;
    struct {
        uint32_t reserved : 29;     /*!< \brief Reserved field */
        uint32_t connected : 1;     /*!< \brief Interface is connected */
        uint32_t rx : 1;            /*!< \brief Interface enabled in RX mode */
        uint32_t tx : 1;            /*!< \brief Interface enabled in TX mode */
    } field;
} axiomreg_ifinfo_t;
/*! \brief Interface enabled in TX mode */
#define AXIOMREG_IFINFO_TX                      0x01
/*! \brief Interface enabled in RX mode */
#define AXIOMREG_IFINFO_RX                      0x02
/*! \brief Interface connected */
#define AXIOMREG_IFINFO_CONNECTED               0x04


/*!
 * \brief AXIOM register: CONTROL bit field
 */
typedef union axiomreg_control {
    uint32_t raw;
    struct {
        uint32_t reserved : 31;     /*!< \brief Reserved field */
        uint32_t loopback : 1;      /*!< \brief Interface in loopback mode */
    } field;
} axiomreg_control_t;
/*! \brief Interface in loopback mode */
#define AXIOMREG_CONTROL_LOOPBACK               0x00000001


/*!
 * \brief AXIOM register: INTERRUPT bit field
 */
typedef union axiomreg_interrupt {
    uint32_t raw;
    struct {
        uint32_t reserved : 28;     /*!< \brief Reserved field */
        uint32_t rdma_rx : 1;       /*!< \brief RDMA RX Queue interrupt */
        uint32_t rdma_tx : 1;       /*!< \brief RDMA TX Queue interrupt */
        uint32_t raw_rx : 1;        /*!< \brief RAW RX Queue interrupt */
        uint32_t raw_tx : 1;        /*!< \brief RAW TX Queue interrupt */
    } field;
} axiomreg_interrupt_t;
/*! \brief RAW TX Queue interrupt */
#define AXIOMREG_IRQ_RAW_TX                     0x00000001
/*! \brief RAW RX Queue interrupt */
#define AXIOMREG_IRQ_RAW_RX                     0x00000002
/*! \brief RDMA TX Queue interrupt */
#define AXIOMREG_IRQ_RDMA_TX                    0x00000004
/*! \brief RDMA RX Queue interrupt */
#define AXIOMREG_IRQ_RDMA_RX                    0x00000008
/*! \brief ALL interrupts */
#define AXIOMREG_IRQ_ALL                        0xFFFFFFFF
/*!
    * \brief AXIOM register: AVLIRQ bit field
 */
typedef union axiomreg_availirq {
    uint32_t raw;
    struct {
        uint32_t rdma_rx : 8;       /*!< \brief RDMA RX avail threashold IRQ */
        uint32_t rdma_tx : 8;       /*!< \brief RDMA TX avail threashold IRQ */
        uint32_t raw_rx : 8;        /*!< \brief RAW RX avail threashold IRQ */
        uint32_t raw_tx : 8;        /*!< \brief RAW TX avail threashold IRQ */
    } field;
} axiomreg_availirq_t;

/*!
    * \brief AXIOM register: QSTATUS bit field
 */
typedef union axiomreg_qstatus {
    uint32_t raw;
    struct {
        uint32_t reserved : 31;     /*!< \brief Reserved field */
        uint32_t avail : 1;         /*!< \brief Slots available in the queue */
    } field;
} axiomreg_qstatus_t;
/*! \brief Slots available in the queue */
#define AXIOMREG_QSTATUS_AVAIL                  0x00000001


/*!
 * \brief AXIOM register: LONG_BUF bit field
 */
typedef union axiomreg_long_buf {
    uint64_t raw;
    struct {
        uint32_t address;           /*!< \brief Address of buffer */
        uint16_t size;              /*!< \brief Size of buffer */
        uint16_t used_msg_id;       /*!< \brief OxFFFF if it is not used,
                                      otherwise it contains the message id */
#define AXIOMREG_LONG_BUF_FREE              0xFFFF
    } field;
} axiomreg_long_buf_t;

/** \} */

#endif /* AXIOM_NIC_REGS_H */
