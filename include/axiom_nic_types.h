#ifndef AXIOM_NIC_TYPES_h
#define AXIOM_NIC_TYPES_h

/*!
 * \file axiom_nic_types.h
 *
 * \version     v0.9
 * \date        2016-06-22
 *
 * This file contains the AXIOM types, return values and flags
 *
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
#define AXIOM_RET_ERROR                 -1
/*! \brief Return value NOTAVAIL: space not available */
#define AXIOM_RET_NOTAVAIL              -2
/*! \brief Return value INTR: system call interrupted */
#define AXIOM_RET_INTR                  -3
/*! \brief Return value NOMEM: memory not available */
#define AXIOM_RET_NOMEM                 -4


/*! \brief Check if return value is OK */
#define AXIOM_RET_IS_OK(_ret)           ((_ret) >= AXIOM_RET_OK)

/******************************* Axiom flags **********************************/
/*! \brief Use no blocking I/O for send/read API */
#define AXIOM_FLAG_NOBLOCK              0x00000001
/*! \brief Avoid flush of RX port queue after the axiom_bind() API */
#define AXIOM_FLAG_NOFLUSH              0x00000002

/*********************** struct/union definitions *****************************/

/*! \brief AXIOM token definition */
union axiom_token {
    uint64_t raw;
    struct {
        uint64_t msg_id : 8;
        uint64_t padding : 24;
        uint64_t value : 32;
    } rdma;
};
#endif /* !AXIOM_NIC_TYPES_h */
