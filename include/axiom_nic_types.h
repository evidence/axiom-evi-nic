#ifndef AXIOM_NIC_API_TYPES_h
#define AXIOM_NIC_API_TYPES_h

/*
 * axiom_nic_api_user.h
 *
 * Version:     v0.3
 * Last update: 2016-03-14
 *
 * This file contains the AXIOM NIC API for the userspace
 *
 */

/****************************** Return Values *********************************/
#define AXIOM_RET_OK            0               /* no error  */
#define AXIOM_RET_ERROR         1               /* error */

/* **************************** Nodes define *********************************/
#define AXIOM_MAX_NODES        255             /* maximim number of nodes */
#define AXIOM_MAX_INTERFACES   4               /* maximum number of interfaces */

/********************************* Types **************************************/
typedef uint8_t	            axiom_port_t;       /* Small message port */
typedef uint8_t	            axiom_flag_t;       /* Small message flag */
typedef uint8_t	            axiom_port_flag_t;  /* Small message port/flag */
typedef uint16_t            axiom_small_len_t;  /* Small queue length */
typedef uint8_t             axiom_node_id_t;    /* Node identifier */
typedef uint8_t             axiom_msg_id_t;     /* Message identifier */
typedef uint8_t	            axiom_if_id_t;      /* Interface identifier */
typedef uint8_t*            axiom_addr_t;       /* Address memory type */
typedef uint32_t            axiom_payload_t;    /* DATA type */
typedef uint8_t	            axiom_err_t;        /* Axiom Error type */
typedef struct axiom_dev    axiom_dev_t;        /* Axiom device private data
                                                 * pointer */
typedef struct axiom_args   axiom_args_t;       /* Axiom open arguments */



#endif /* !AXIOM_NIC_API_TYPES_h */
