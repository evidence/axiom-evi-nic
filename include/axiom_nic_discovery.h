#ifndef AXIOM_NIC_DISCOVERY_h
#define AXIOM_NIC_DISCOVERY_h

/*
 * axiom_nic_discovery.h
 *
 * Last update: 2016-03-08
 *
 * This file contains the AXIOM NIC API for the discovery
 *
 */

/********************************* Types **************************************/
typedef uint8_t		axiom_raw_disc_type_t;	/* Raw message discovery-type */

/*
 * @brief This function sends a discovery message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_raw_discovery(axiom_dev_t *dev, axiom_if_id_t src_local_if,
        axiom_raw_disc_type_t data_type,
        axiom_node_id_t data_src_id, axiom_node_id_t data_dst_id,
        axiom_if_id_t data_src_if, axiom_if_id_t data_dst_if)
{
    axiom_discovery_msg_t msg;
    axiom_msg_id_t ret;


    msg.data.disc.type = data_type;
    msg.data.disc.src_node = data_src_id;
    msg.data.disc.dst_node = data_dst_id;
    msg.data.disc.src_dst_if  = ((axiom_if_id_t)((0x0F & data_src_if) << 4) |
                                (0x0F & data_dst_if)) ;


    ret = axiom_send_raw_neighbour(dev, src_local_if, AXIOM_RAW_TYPE_DISCOVERY, msg.data.raw);

    return ret;
}

/*
 * @brief This function sends a discovery message to a neighbour on a specific
 *        interface.
 * @param dev The axiom devive private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_recv_raw_discovery(axiom_dev_t *dev, axiom_raw_disc_type_t *data_type,
                         axiom_node_id_t *src_id, axiom_node_id_t *dst_id,
                         axiom_if_id_t *src_interface, axiom_if_id_t *dst_interface,
                         axiom_if_id_t *data_src_if, axiom_if_id_t *data_dst_if)
{
    axiom_discovery_msg_t msg;
    axiom_msg_id_t ret;

    ret = axiom_recv_raw_neighbour (dev, &msg.header.neighbour.src_if,
            &msg.header.neighbour.dst_if, &msg.header.neighbour.type,
            &msg.data.raw);

    if ((ret == AXIOM_RET_OK) &&
        (msg.header.neighbour.type == AXIOM_RAW_TYPE_DISCOVERY))
    {
        /* Header info */
        *src_interface = msg.header.neighbour.src_if;
        *dst_interface = msg.header.neighbour.dst_if;
        /* payload info */
        *data_type = msg.data.disc.type;
        *src_id    = msg.data.disc.src_node;
        *dst_id    = msg.data.disc.dst_node;
        *data_src_if = (msg.data.disc.src_dst_if >> 4) & 0x0F;
        *data_dst_if = (msg.data.disc.src_dst_if) & 0x0F;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}

#endif /* !AXIOM_NIC_DISCOVERY_h */
