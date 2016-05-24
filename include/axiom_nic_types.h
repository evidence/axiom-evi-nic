#ifndef AXIOM_NIC_TYPES_h
#define AXIOM_NIC_TYPES_h

/*!
 * \file axiom_nic_types.h
 *
 * \version     v0.5
 * \date        2016-04-08
 *
 * This file contains the AXIOM types
 *
 */


/****************************** Return Values *********************************/
/*! \brief Return value OK */
#define AXIOM_RET_OK            0
/*! \brief Return value ERROR */
#define AXIOM_RET_ERROR         1


/* **************************** Nodes define *********************************/
/*! \brief Maximum number of nodes supported by the AXIOM NIC */
#define AXIOM_MAX_NODES        255
/*! \brief Maximum number of interfaces supported by the AXIOM NIC */
#define AXIOM_MAX_INTERFACES   4


/********************************* Types **************************************/
/*! \brief AXIOM port of SMALL messages */
typedef uint8_t	            axiom_port_t;
/*! \brief AXIOM type of SMALL messages */
typedef uint8_t	            axiom_type_t;
/*! \brief AXIOM port/type of SMALL messages */
typedef uint8_t	            axiom_port_type_t;
/*! \brief AXIOM queue length type */
typedef uint32_t            axiom_small_len_t;
/*! \brief AXIOM node identifier */
typedef uint8_t             axiom_node_id_t;
/*! \brief AXIOM message identifier */
typedef uint8_t             axiom_msg_id_t;
/*! \brief AXIOM interface identifier */
typedef uint8_t	            axiom_if_id_t;
/*! \brief AXIOM address memory */
typedef uint8_t*            axiom_addr_t;
/*! \brief AXIOM payload size type */
typedef uint8_t             axiom_payload_size_t;
/*! \brief AXIOM error type */
typedef uint8_t	            axiom_err_t;
/*! \brief AXIOM device private data */
typedef struct axiom_dev    axiom_dev_t;
/*! \brief AXIOM open arguments */
typedef struct axiom_args   axiom_args_t;



#endif /* !AXIOM_NIC_TYPES_h */
