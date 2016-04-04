#ifndef AXIOM_NIC_SETROUTE_H
#define AXIOM_NIC_SETROUTE_H

/*
 * axiom_nic_set_route.h
 *
 * Last update: 2016-03-10
 *
 * This file contains the AXIOM NIC API for the set routing phase
 *
 */

/***************************** Types ***************************/
/* Raw message set_route-type */
typedef uint8_t		axiom_raw_set_route_type_t;

/*
 * @brief This function sends a set-routing message to a neighbour
 *        on a specific interface.
 * @param dev The axiom device private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_send_raw_set_routing(axiom_dev_t *dev, axiom_if_id_t src_local_if,
                            axiom_raw_set_route_type_t data_type)
{
    axiom_set_routing_msg_t msg;
    axiom_msg_id_t ret;

    msg.data.set_routing.type = data_type;

    ret = axiom_send_raw_neighbour(dev, src_local_if,
                             AXIOM_RAW_TYPE_ROUTING, msg.data.raw);

    return ret;
}

/*
 * @brief This function sends a set-routing message to a neighbour
 *        on a specific interface.
 * @param dev The axiom device private data pointer
 * @param src_interface Sender interface identification
 * @param type type of the raw message
 * @param data Data to send
 * return Returns ...
 */
inline static axiom_msg_id_t
axiom_recv_raw_set_routing(axiom_dev_t *dev, axiom_if_id_t *src_local_if,
                            axiom_raw_set_route_type_t *data_type)
{
    axiom_set_routing_msg_t msg;
    axiom_msg_id_t ret;

    ret = axiom_recv_raw_neighbour (dev, &msg.header.neighbour.src_if,
            &msg.header.neighbour.dst_if, &msg.header.neighbour.type,
            &msg.data.raw);

    if ((ret == AXIOM_RET_OK) && (msg.header.neighbour.type == AXIOM_RAW_TYPE_ROUTING))
    {
        /* Header info */
        *src_local_if = msg.header.neighbour.src_if;

        /* payload info */
        *data_type =  msg.data.set_routing.type;

        return AXIOM_RET_OK;
    }

    return AXIOM_RET_ERROR;
}

#endif /* !AXIOM_NIC_DISCOVERY_h */
