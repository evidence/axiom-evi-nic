/*!
 * \file axiom_nic_types.h
 *
 * \version     v1.0
 * \date        2016-06-22
 *
 * This file contains the AXIOM types, return values and flags
 *
 * Copyright (C) 2016, Evidence Srl.
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NIC_TYPES_h
#define AXIOM_NIC_TYPES_h

/**
 * \defgroup AXIOM_NIC
 *
 * \{
 */

/********************************* Types **************************************/
/*! \brief AXIOM port of messages */
typedef uint8_t	            axiom_port_t;
/*! \brief AXIOM type of messages */
typedef uint8_t	            axiom_type_t;
/*! \brief AXIOM queue length type */
typedef uint32_t            axiom_queue_len_t;
/*! \brief AXIOM node identifier */
typedef uint8_t             axiom_node_id_t;
/*! \brief AXIOM application identifier */
typedef uint8_t             axiom_app_id_t;
/*! \brief AXIOM message identifier */
typedef uint8_t             axiom_msg_id_t;
/*! \brief AXIOM interface identifier */
typedef uint8_t	            axiom_if_id_t;
/*! \brief AXIOM address memory */
typedef uint32_t            axiom_addr_t;
/*! \brief AXIOM payload size type for RAW messages */
typedef uint8_t             axiom_raw_payload_size_t;
/*! \brief AXIOM payload size type for LONG messages */
typedef uint16_t            axiom_long_payload_size_t;
/*! \brief AXIOM payload size type for RDMA messages */
typedef uint16_t            axiom_rdma_payload_size_t;
/*! \brief AXIOM error type */
typedef int	            axiom_err_t;
/*! \brief AXIOM device private data */
typedef struct axiom_dev    axiom_dev_t;
/*! \brief AXIOM open arguments */
typedef struct axiom_args   axiom_args_t;
/*! \brief AXIOM flags */
typedef uint64_t            axiom_flags_t;
/*! \brief AXIOM token */
typedef union axiom_token   axiom_token_t;
/*! \brief AXIOM statistics */
typedef struct axiom_stats  axiom_stats_t;

/*! \brief Invalid node ID */
#define AXIOM_NULL_NODE                 255
/*! \brief Invalid node ID */
#define AXIOM_NULL_APP_ID               255
/*! \brief Ask any port number during the bind */
#define AXIOM_PORT_ANY                  255

/****************************** Return Values *********************************/
/*! \brief Return value OK */
#define AXIOM_RET_OK                    0
/*! \brief Return value ERROR: generic error */
#define AXIOM_RET_ERROR                 (-1)
/*! \brief Return value NOTAVAIL: space not available */
#define AXIOM_RET_NOTAVAIL              (-2)
/*! \brief Return value INTR: system call interrupted */
#define AXIOM_RET_INTR                  (-3)
/*! \brief Return value NOMEM: memory not available */
#define AXIOM_RET_NOMEM                 (-4)
/*! \brief Return value NOTREACH: node not reachable */
#define AXIOM_RET_NOTREACH              (-5)


/*! \brief Check if return value is OK */
#define AXIOM_RET_IS_OK(_ret)           ((_ret) >= AXIOM_RET_OK)

/******************************* Axiom flags **********************************/
/*! \brief Use no blocking I/O for send/recv RAW API */
#define AXIOM_FLAG_NOBLOCK_RAW          0x00000001
/*! \brief Use no blocking I/O for send/recv LONG API */
#define AXIOM_FLAG_NOBLOCK_LONG         0x00000002
/*! \brief Use no blocking I/O for write/read RDMA API */
#define AXIOM_FLAG_NOBLOCK_RDMA         0x00000004
/*! \brief Use no blocking I/O for send/write recv/read API */
#define AXIOM_FLAG_NOBLOCK              0x00000007
/*! \brief Avoid flush of RX port queue after the axiom_bind() API */
#define AXIOM_FLAG_NOFLUSH              0x00000008


/**************************** Axiom debug flags *******************************/
#define AXIOM_DEBUG_FLAG_STATUS         0x0000001
#define AXIOM_DEBUG_FLAG_CONTROL        0x0000002
#define AXIOM_DEBUG_FLAG_ROUTING        0x0000004
#define AXIOM_DEBUG_FLAG_QUEUES         0x0000008
#define AXIOM_DEBUG_FLAG_RAW            0x0000010
#define AXIOM_DEBUG_FLAG_LONG           0x0000020
#define AXIOM_DEBUG_FLAG_RDMA           0x0000040
#define AXIOM_DEBUG_FLAG_FPGA           0x0000080

#define AXIOM_DEBUG_FLAG_ALL            0xFFFFFFF

/*************************** Axiom token status  *******************************/
/*! \brief AXIOM token invalid status */
#define AXIOM_TOKEN_INVALID             0
/*! \brief AXIOM token pending status */
#define AXIOM_TOKEN_PENDING             1
/*! \brief AXIOM token acked status */
#define AXIOM_TOKEN_ACKED               2

/*********************** struct/union definitions *****************************/
/*! \brief AXIOM NIC statistics */
struct axiom_stats {
    /*! \brief Interrupts received (total and for each queues) */
    uint64_t irq;
    uint64_t irq_raw_tx;
    uint64_t irq_raw_rx;
    uint64_t irq_rdma_tx;
    uint64_t irq_rdma_rx;

    /*! \brief Packets sent and received
     * (RDMA RX is used only for acks and RDMA TX counts also LONG TX) */
    uint64_t pkt_raw_tx;
    uint64_t pkt_raw_rx;
    uint64_t pkt_long_tx;
    uint64_t pkt_long_rx;
    uint64_t pkt_rdma_tx;
    uint64_t pkt_rdma_rx;

    /*! \brief Bytes sent and received */
    uint64_t bytes_raw_tx;
    uint64_t bytes_raw_rx;
    uint64_t bytes_long_tx;
    uint64_t bytes_long_rx;
    uint64_t bytes_rdma_tx;
    uint64_t bytes_rdma_rx;

    /*! \brief Error occured during send and receive */
    uint64_t err_raw_tx;
    uint64_t err_raw_rx;
    uint64_t err_long_tx;
    uint64_t err_long_rx;
    uint64_t err_rdma_tx;
    uint64_t err_rdma_rx;

    /*! \brief Number of syscall blocked waiting new slots available */
    uint64_t wait_raw_tx;
    uint64_t wait_raw_rx;
    uint64_t wait_long_tx;
    uint64_t wait_long_rx;
    uint64_t wait_rdma_tx;
    uint64_t wait_rdma_rx;

    /*! \brief Number of poll() requests */
    uint64_t poll_raw_tx;
    uint64_t poll_raw_rx;
    uint64_t poll_long_tx;
    uint64_t poll_long_rx;
    uint64_t poll_rdma_tx;
    uint64_t poll_rdma_rx;

    /*! \brief Number of poll() requests where the space is available */
    uint64_t poll_avail_raw_tx;
    uint64_t poll_avail_raw_rx;
    uint64_t poll_avail_long_tx;
    uint64_t poll_avail_long_rx;
    uint64_t poll_avail_rdma_tx;
    uint64_t poll_avail_rdma_rx;

    /*! \brief Number of RDMA/LONG packets retransmit */
    uint64_t retries_rdma;
    /*! \brief Number of RDMA/LONG packets discarded */
    uint64_t discarded_rdma;
};

/*! \brief AXIOM token definition */
union axiom_token {
    uint64_t raw;
    struct {
        uint64_t msg_id : 8;    /*!< \brief message ID */
        uint64_t status : 8;    /*!< \brief token status */
        uint64_t padding : 16;
        uint64_t value : 32;    /*!< \brief token value */
    } rdma;
};

/*!
 * \brief Invalidate a given token
 *
 * \param _tkn          AXIOM token to invalidate
 */
#define AXIOM_TOKEN_INVALIDATE(_tkn)    \
    ((_tkn)->rdma.status = AXIOM_TOKEN_INVALID)

/*!
 * \brief Check if token is valid
 *
 * \param _tkn          AXIOM token to check
 *
 * \return Returns true if tokend is valid, false otherwise.
 */
#define AXIOM_TOKEN_IS_VALID(_tkn)      \
    ((_tkn)->rdma.status != AXIOM_TOKEN_INVALID)

/*!
 * \brief Check if token is pending
 *
 * \param _tkn          AXIOM token to check
 *
 * \return Returns true if tokend is pending, false otherwise.
 */
#define AXIOM_TOKEN_IS_PENDING(_tkn)    \
    ((_tkn)->rdma.status == AXIOM_TOKEN_PENDING)

/*!
 * \brief Check if token is acked
 *
 * \param _tkn          AXIOM token to check
 *
 * \return Returns true if tokend is acked, false otherwise.
 */
#define AXIOM_TOKEN_IS_ACKED(_tkn)    \
    ((_tkn)->rdma.status == AXIOM_TOKEN_ACKED)


/** \} */

#endif /* !AXIOM_NIC_TYPES_h */
