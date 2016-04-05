#ifndef AXIOM_SWITCH_QEMU_h
#define AXIOM_SWITCH_QEMU_h

#include "axiom_nic_regs.h"

inline static int
axsw_qemu_send(int fd, axiom_small_eth_t *small_eth)
{
    int ret;
    uint32_t axiom_msg_length = htonl(sizeof(*small_eth));

    /* send the length of the ethernet packet */
    ret = send(fd, &axiom_msg_length, sizeof(axiom_msg_length), 0);
    if (ret != sizeof(axiom_msg_length)) {
        EPRINTF("length send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    /* send ethernet packet */
    ret = send(fd, small_eth, sizeof(*small_eth), 0);
    if (ret != sizeof(*small_eth))
    {
        EPRINTF("message send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    return ret;
}


inline static int
axsw_qemu_recv(int fd, axiom_small_eth_t *small_eth)
{
    int ret;
    uint32_t axiom_msg_length;

    /* receive the length of the ethernet packet */
    ret = recv(fd, &axiom_msg_length, sizeof(axiom_msg_length), MSG_WAITALL);
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
    ret = recv(fd, small_eth, axiom_msg_length, MSG_WAITALL);
    if (ret < 0 && (errno == EWOULDBLOCK)) {
        goto skip;
    } else if (ret <= 0) {
        goto err;
    }

    if (ret != axiom_msg_length) {
        EPRINTF("unexpected length - expected: %d received: %d", axiom_msg_length, ret);
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

/* This function send to qemu the interface information */
inline static int
axsw_qemu_send_ifinfo(int fd, axiom_if_id_t if_id, int connected)
{
    axiom_small_eth_t small_eth;

    small_eth.eth_hdr.type = htons(AXIOM_ETH_TYPE_CTRL);
    memset(&small_eth, 0, sizeof(small_eth));

    /* send if_info to QEMU vm:
     *  - header.src contains the interface id
     *  - payload[0] contains the information
     */
    small_eth.small_msg.header.tx.dst = if_id;
    if (connected) {
        ((uint8_t *)&small_eth.small_msg.payload)[0] =
            AXIOMREG_IFINFO_CONNECTED | AXIOMREG_IFINFO_RX | AXIOMREG_IFINFO_TX;
    }

    return axsw_qemu_send(fd, &small_eth);
}

#endif /* !AXIOM_SWITCH_QEMU_h */
