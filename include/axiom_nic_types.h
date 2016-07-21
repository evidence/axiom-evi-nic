#ifndef AXIOM_NIC_TYPES_h
#define AXIOM_NIC_TYPES_h

/*!
 * \file axiom_nic_types.h
 *
 * \version     v0.6
 * \date        2016-06-22
 *
 * This file contains the AXIOM types, return values and flags
 *
 */

/********************************* Types **************************************/
/*! \brief AXIOM port of RAW messages */
typedef uint8_t	            axiom_port_t;
/*! \brief AXIOM type of RAW messages */
typedef uint8_t	            axiom_type_t;
/*! \brief AXIOM queue length type */
typedef uint32_t            axiom_queue_len_t;
/*! \brief AXIOM node identifier */
typedef uint8_t             axiom_node_id_t;
/*! \brief AXIOM message identifier */
typedef uint8_t             axiom_msg_id_t;
/*! \brief AXIOM interface identifier */
typedef uint8_t	            axiom_if_id_t;
/*! \brief AXIOM address memory */
typedef uint32_t            axiom_addr_t;
/*! \brief AXIOM payload size type for RAW messages */
typedef uint8_t             axiom_raw_payload_size_t;
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

/*! \brief Invalid node ID */
#define AXIOM_NULL_NODE                 255
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


/*! \brief Check if return value is OK */
#define AXIOM_RET_IS_OK(_ret)           ((_ret) >= AXIOM_RET_OK)

/******************************* Axiom flags **********************************/
/*! \brief Use no blocking I/O for send/read API */
#define AXIOM_FLAG_NOBLOCK              0x00000001
/*! \brief Avoid flush of RX port queue after the axiom_bind() API */
#define AXIOM_FLAG_NOFLUSH              0x00000002


#endif /* !AXIOM_NIC_TYPES_h */
