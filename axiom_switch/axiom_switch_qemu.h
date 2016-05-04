#ifndef AXIOM_SWITCH_QEMU_h
#define AXIOM_SWITCH_QEMU_h
/*!
 * \file axiom_switch_qemu.h
 *
 * \version     v0.4
 * \date        2016-05-03
 *
 * This file contains the interface to send/receive message to/from QEMU socket
 * backend.
 */

#include "axiom_nic_regs.h"


/*!
 * \brief Send packet to QEMU socket backend.
 *
 * The QEMU socket backend expects to receive 4 bytes (uint32_t) of packet
 * length before to receive the packet.
 *
 * \param sd            socket descriptor connected to QEMU socket backend
 * \param small_eth     packet to send
 *
 * \return send() return value (bytes sent on success, -1 on error)
 */
inline static int
axsw_qemu_send(int sd, axiom_small_eth_t *small_eth)
{
    int ret;
    uint32_t axiom_msg_length = htonl(sizeof(*small_eth));

    /* send the length of the ethernet packet */
    ret = send(sd, &axiom_msg_length, sizeof(axiom_msg_length), 0);
    if (ret != sizeof(axiom_msg_length)) {
        EPRINTF("length send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    /* send ethernet packet */
    ret = send(sd, small_eth, sizeof(*small_eth), 0);
    if (ret != sizeof(*small_eth)) {
        EPRINTF("message send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    return ret;
}


/*!
 * \brief Receive packet from QEMU socket backend.
 *
 * The QEMU socket backend sends 4 bytes (uint32_t) of packet length before to
 * send the packet.
 *
 * \param sd            socket descriptor connected to QEMU socket backend
 * \param small_eth     packet received
 *
 * \return recv() return value (bytes received on success, -1 on error)
 */
inline static int
axsw_qemu_recv(int sd, axiom_small_eth_t *small_eth)
{
    int ret;
    uint32_t axiom_msg_length;

    /* receive the length of the ethernet packet */
    ret = recv(sd, &axiom_msg_length, sizeof(axiom_msg_length), MSG_WAITALL);
    if (ret < 0 && (errno == EWOULDBLOCK)) {
        goto skip;
    } else if (ret <= 0) {
        goto err;
    }

    axiom_msg_length = ntohl(axiom_msg_length);
    if (axiom_msg_length > sizeof(*small_eth)) {
        EPRINTF("too long message - len: %d", axiom_msg_length);
        goto skip;
    }

    /* receive ethernet packet */
    ret = recv(sd, small_eth, axiom_msg_length, MSG_WAITALL);
    if (ret < 0 && (errno == EWOULDBLOCK)) {
        goto skip;
    } else if (ret <= 0) {
        goto err;
    }

    if (ret != axiom_msg_length) {
        EPRINTF("unexpected length - expected: %d received: %d",
                axiom_msg_length, ret);
        goto skip;
    }


    return axiom_msg_length;

skip:
    return 0;
err:
    if (ret)
        EPRINTF("connection error ret: %d - errno: %d", ret, errno);
    return -1;
}

/*!
 * \brief Send interface information to Axiom-NIC emulation through QEMU socket
 * backend.
 *
 * \param sd            socket descriptor connected to QEMU socket backend
 * \param if_id         interface identifier
 * \param connected     interface status
 *
 * \return axsw_qemu_send() return value (bytes sent on success, -1 on error)
 */
inline static int
axsw_qemu_send_ifinfo(int sd, axiom_if_id_t if_id, int connected)
{
    axiom_small_eth_t small_eth;

    memset(&small_eth, 0, sizeof(small_eth));
    small_eth.eth_hdr.type = htons(AXIOM_ETH_TYPE_CTRL);

    /* send if_info to QEMU vm:
     *  - header.src contains the interface id
     *  - payload[0] contains the information
     */
    small_eth.small_msg.header.tx.dst = if_id;
    if (connected) {
        ((uint8_t *)&small_eth.small_msg.payload)[0] =
            AXIOMREG_IFINFO_CONNECTED | AXIOMREG_IFINFO_RX | AXIOMREG_IFINFO_TX;
    }
    DPRINTF("if_id: %x if_info: %x", if_id,
            ((uint8_t *)&small_eth.small_msg.payload)[0]);

    return axsw_qemu_send(sd, &small_eth);
}

#endif /* !AXIOM_SWITCH_QEMU_h */
