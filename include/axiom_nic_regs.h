/*!
 * \file axiom_nic_regs.h
 *
 * \version     v0.15
 * \date        2016-03-18
 *
 * This file contains the following AXIOM NIC registers informations:
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
/* TODO: update to 4 byte */
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
        uint32_t reserved : 30;     /*!< \brief Reserved field */
        uint32_t xsmll_enable : 1;  /*!< \brief Enable/disable XSMLL */
        uint32_t ack_enable : 1;    /*!< \brief Enable/disable ACK */
        uint32_t reset : 1;         /*!< \brief Reset all Aurora IPs */
        uint32_t loopback : 1;      /*!< \brief Enable external loopback */
    } field;
} axiomreg_control_t;
/*! \brief Enable external loopback */
#define AXIOMREG_CONTROL_LOOPBACK               0x00000001
/*! \brief Reset all Aurora IPs */
#define AXIOMREG_CONTROL_RESET                  0x00000002
/*! \brief Enable/disable ACK */
#define AXIOMREG_CONTROL_ACK_ENABLE             0x00000004
/*! \brief Enable/disable XSMLL */
#define AXIOMREG_CONTROL_XSMLL_ENABLE           0x00000008


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

/*! \brief AXIOM ROUTING LOOPBACK interface */
#define AXIOMREG_ROUTING_LOOPBACK_IF            0x0
/*! \brief AXIOM ROUTING NULL interface */
#define AXIOMREG_ROUTING_NULL_IF                0xFF

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
        uint32_t address;   /*!< \brief Address of buffer */
        uint16_t size;      /*!< \brief Size of buffer */
        uint8_t msg_id;     /*!< \brief ID of message that uses this buffer */
        uint8_t flags;
#define AXIOMREG_LONG_BUF_FREE              0x01
    } field;
} axiomreg_long_buf_t;

/** \} */

#endif /* AXIOM_NIC_REGS_H */
