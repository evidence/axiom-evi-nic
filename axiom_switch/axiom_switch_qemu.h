#ifndef AXIOM_SWITCH_QEMU_h
#define AXIOM_SWITCH_QEMU_h


inline static int
axsw_qemu_send(int fd, axiom_raw_eth_t *raw_eth)
{
    int ret;
    uint32_t axiom_msg_length = 0;

    axiom_msg_length = sizeof(*raw_eth);


    /* send the length of the ethernet packet */
    ret = send(fd, &axiom_msg_length, sizeof(axiom_msg_length), 0);
    if (ret != sizeof(axiom_msg_length)) {
        EPRINTF("length send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    /* send ethernet packet */
    ret = send(fd, raw_eth, axiom_msg_length, 0);
    if (ret != axiom_msg_length)
    {
        DPRINTF("message send error - return: %d errno: %d", ret, errno);
        return -1;
    }

    return ret;
}


inline static int
axsw_qemu_recv(int fd, axiom_raw_eth_t *raw_eth)
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
    if (axiom_msg_length > sizeof(*raw_eth)) {
        EPRINTF("too long message - len: %d", axiom_msg_length);
        goto skip;
    }

    /* receive ethernet packet */
    ret = recv(fd, raw_eth, axiom_msg_length, MSG_WAITALL);
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
    EPRINTF("connection error - errno: %d", errno);
    return -1;
}

#endif /* !AXIOM_SWITCH_QEMU_h */
