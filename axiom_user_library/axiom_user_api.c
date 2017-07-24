/*!
 * \file axiom_user_api.c
 *
 * \version     v0.13
 * \date        2016-05-03
 *
 * This file contains the implementation of Axiom NIC API for the user-space
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "dprintf.h"
#include "axiom_nic_api_user.h"
#include "axiom_nic_packets.h"
#include "axiom_netdev_user.h"
#include "axiom_utility.h"

#ifdef AXIOM_EXTRAE_SUPPORT
#include <extrae_types.h>
#include <extrae_user_events.h>
static extrae_type_t axiom_extrae_apinic = 9990000;
static int axiom_extrae_apinic_init = 0;

typedef enum {
    AX_EXTRAE_APINIC_END,
    AX_EXTRAE_APINIC_OPEN,
    AX_EXTRAE_APINIC_CLOSE,
    AX_EXTRAE_APINIC_SEND,
    AX_EXTRAE_APINIC_SEND_IOV,
    AX_EXTRAE_APINIC_RECV,
    AX_EXTRAE_APINIC_RECV_IOV,
    AX_EXTRAE_APINIC_SEND_RAW,
    AX_EXTRAE_APINIC_SEND_IOV_RAW,
    AX_EXTRAE_APINIC_RECV_RAW,
    AX_EXTRAE_APINIC_RECV_IOV_RAW,
    AX_EXTRAE_APINIC_SEND_RAW_AVAIL,
    AX_EXTRAE_APINIC_RECV_RAW_AVAIL,
    AX_EXTRAE_APINIC_SEND_LONG,
    AX_EXTRAE_APINIC_SEND_IOV_LONG,
    AX_EXTRAE_APINIC_RECV_LONG,
    AX_EXTRAE_APINIC_RECV_IOV_LONG,
    AX_EXTRAE_APINIC_SEND_LONG_AVAIL,
    AX_EXTRAE_APINIC_RECV_LONG_AVAIL,
    AX_EXTRAE_APINIC_RDMA_READ,
    AX_EXTRAE_APINIC_RDMA_WRITE,
    AX_EXTRAE_APINIC_RDMA_CHECK,
    AX_EXTRAE_APINIC_RDMA_WAIT,
    AX_EXTRAE_APINIC_LAST
} axiom_extrae_apinic_t;

char* axiom_extrae_apinic_desc[] = {
    "axiom_open()",
    "axiom_close()",
    "axiom_send()",
    "axiom_send_iov()",
    "axiom_recv()",
    "axiom_recv_iov()",
    "axiom_send_raw()",
    "axiom_send_iov_raw()",
    "axiom_recv_raw()",
    "axiom_recv_iov_raw()",
    "axiom_send_raw_avail()",
    "axiom_recv_raw_avail()",
    "axiom_send_long()",
    "axiom_send_iov_long()",
    "axiom_recv_long()",
    "axiom_recv_iov_long()",
    "axiom_send_long_avail()",
    "axiom_recv_long_avail()",
    "axiom_rdma_read()",
    "axiom_rdma_write()",
    "axiom_rdma_check()",
    "axiom_rdma_wait()",
};

void axiom_extrae_init(extrae_type_t *type, char *name, char **val_desc,
        unsigned val_num, int *initialized) {
    if (!(*initialized) && Extrae_is_initialized()) {
        extrae_value_t *values = malloc(sizeof(*values) *val_num);
        int i;

        for (i = 0; i < val_num; i++)
            values[i] = i + 1;

        Extrae_define_event_type(type, name, &val_num, values, val_desc);

        IPRINTF(1, "%s - extrae initialized", name);

        free(values);

        *initialized = 1;
    }
}
#define AXIOM_EXTRAE(f)                                                 \
    do {                                                                \
        axiom_extrae_init(&axiom_extrae_apinic, "AXIOM NIC API",        \
                axiom_extrae_apinic_desc, AX_EXTRAE_APINIC_LAST - 1,    \
                &axiom_extrae_apinic_init);                             \
        f;                                                              \
    } while(0);
#else
#define AXIOM_EXTRAE(f)
#endif

/*! \brief Axiom char dev default name */
#define AXIOM_DEV_NAME          "/dev/axiom"
#define AXIOM_DEV_RAW_NAME      "/dev/axiom-raw"
#define AXIOM_DEV_LONG_NAME     "/dev/axiom-long"
#define AXIOM_DEV_RDMA_NAME     "/dev/axiom-rdma"

#define AXIOM_NUM_RECV_FDS      2

#define AXIOM_RDMA_DEBUG

/*!
 * \brief axiom arguments for the axiom_open() function
 */
typedef struct axiom_dev {
    int fd_generic;      /*!< \brief fd of the AXIOM generic char dev */
    int fd_raw;          /*!< \brief fd of the AXIOM raw char dev */
    int fd_long;         /*!< \brief fd of the AXIOM long char dev */
    int fd_rdma;         /*!< \brief fd of the AXIOM rdma char dev */
    struct pollfd recv_fds[AXIOM_NUM_RECV_FDS];
    axiom_flags_t flags; /*!< \brief axiom flags */
    void *rdma_addr;     /*!< \brief rdma zone pointer */
    uint64_t rdma_size;  /*!< \brief rdma zone size */
    int appid;           /*!< \brief application ID to use in the RDMA */
} axiom_dev_t;


static int
axiom_get_appid(void)
{
    char *appid_s;
    unsigned long appid = 0;

    appid_s = getenv(AXIOM_ENV_ALLOC_APPID);
    if (appid_s == NULL) {
        return AXIOM_RET_ERROR;
    }

    appid = strtoul(appid_s, NULL, 10);

    return (axiom_app_id_t)(appid);
}

axiom_dev_t *
axiom_open(axiom_args_t *args) {
    axiom_dev_t *dev;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_OPEN));

    dev = calloc(1, sizeof(*dev));
    if (!dev) {
        EPRINTF("failed to allocate memory");
        return NULL;
    }

    dev->fd_generic = open(AXIOM_DEV_NAME, O_RDWR);
    if (dev->fd_generic < 0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_NAME);
        goto free_dev;
    }

    dev->fd_raw = open(AXIOM_DEV_RAW_NAME, O_RDWR);
    if (dev->fd_raw < 0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_RAW_NAME);
        goto close_fd_gen;
    }
    dev->recv_fds[0].fd = dev->fd_raw;
    dev->recv_fds[0].events = POLLIN;

    dev->fd_long = open(AXIOM_DEV_LONG_NAME, O_RDWR);
    if (dev->fd_long < 0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_LONG_NAME);
        goto close_fd_raw;
    }
    dev->recv_fds[1].fd = dev->fd_long;
    dev->recv_fds[1].events = POLLIN;

    dev->fd_rdma = open(AXIOM_DEV_RDMA_NAME, O_RDWR);
    if (dev->fd_rdma < 0) {
        EPRINTF("impossible to open %s", AXIOM_DEV_RDMA_NAME);
        goto close_fd_long;
    }

    if (args) {
        axiom_err_t ret;

        ret = axiom_set_flags(dev, args->flags);
        if (ret) {
            EPRINTF("impossible to set flags");
            goto close_fd_rdma;
        }
    }

    dev->appid = axiom_get_appid();

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return dev;

close_fd_rdma:
    close(dev->fd_rdma);
close_fd_long:
    close(dev->fd_long);
close_fd_raw:
    close(dev->fd_raw);
close_fd_gen:
    close(dev->fd_generic);
free_dev:
    free(dev);
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return NULL;
}

void
axiom_close(axiom_dev_t *dev)
{
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_CLOSE));

    if (!dev)
        return;

    close(dev->fd_rdma);
    close(dev->fd_long);
    close(dev->fd_raw);
    close(dev->fd_generic);
    free(dev);

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
}

static axiom_err_t
axiom_fcntl(int fd, int flag, int set)
{
    int fd_flags;

    fd_flags = fcntl(fd, F_GETFL);
    if (fd_flags == -1) {
        EPRINTF("fcntl error - errno: %s", strerror(errno));
        return AXIOM_RET_ERROR;
    }

    if (set)
        fd_flags |= flag;
    else
        fd_flags &= ~flag;

    if (fcntl(fd, F_SETFL, fd_flags) == -1) {
        EPRINTF("fcntl error - errno: %s", strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

static axiom_err_t
axiom_update_flags(axiom_dev_t *dev, axiom_flags_t update_flags)
{
    axiom_err_t ret = 0;

    /* check if flags are set or not */

    if (update_flags & AXIOM_FLAG_NOBLOCK_RAW) {
        ret |= axiom_fcntl(dev->fd_raw, O_NONBLOCK,
                dev->flags & AXIOM_FLAG_NOBLOCK_RAW);
    }

    if (update_flags & AXIOM_FLAG_NOBLOCK_LONG) {
        ret |= axiom_fcntl(dev->fd_long, O_NONBLOCK,
                dev->flags & AXIOM_FLAG_NOBLOCK_LONG);
    }

    if (update_flags & AXIOM_FLAG_NOBLOCK_RDMA) {
        ret |= axiom_fcntl(dev->fd_rdma, O_NONBLOCK,
                dev->flags & AXIOM_FLAG_NOBLOCK_RDMA);
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_set_flags(axiom_dev_t *dev, axiom_flags_t flags)
{
    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    dev->flags |= flags;

    return axiom_update_flags(dev, flags);
}

axiom_err_t
axiom_unset_flags(axiom_dev_t *dev, axiom_flags_t flags)
{
    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    dev->flags &= ~flags;

    return axiom_update_flags(dev, flags);
}

axiom_err_t
axiom_get_fds(axiom_dev_t *dev, int *raw_fd, int *long_fd, int *rdma_fd)
{
    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    if (raw_fd) {
        *raw_fd = dev->fd_raw;
    }
    if (long_fd) {
        *long_fd = dev->fd_long;
    }
    if (rdma_fd) {
        *rdma_fd = dev->fd_rdma;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_bind(axiom_dev_t *dev, axiom_port_t port)
{
    axiom_ioctl_bind_t ioctl_bind;
    int ret;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ioctl_bind.port = port;
    ioctl_bind.flush = (dev->flags & AXIOM_FLAG_NOFLUSH) ? 0 : 1;

    if (dev->fd_raw >= 0) {
        ret = ioctl(dev->fd_raw, AXNET_BIND, &ioctl_bind);
        if (ret < 0) {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            return AXIOM_RET_ERROR;
        }
    }

    if (dev->fd_long >= 0) {
        ret = ioctl(dev->fd_long, AXNET_BIND, &ioctl_bind);
        if (ret < 0) {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            return AXIOM_RET_ERROR;
        }
    }

    return ioctl_bind.port;
}

axiom_err_t
axiom_next_hop(axiom_dev_t *dev, axiom_node_id_t dst_id,
               axiom_if_id_t *if_number) {
    axiom_err_t ret;
    uint8_t enabled_mask;
    int i;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = axiom_get_routing(dev, dst_id, &enabled_mask);
    if (!AXIOM_RET_IS_OK(ret))
        return ret;

    for (i = 0; i < AXIOM_INTERFACES_MAX; i++) {
        if (enabled_mask & (uint8_t)(1 << i)) {
            *if_number = i;
            return AXIOM_RET_OK;
        }
    }
    return AXIOM_RET_ERROR;
}

axiom_err_t
axiom_send(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        size_t payload_size, void *payload)
{
    axiom_err_t ret;
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_SEND));

    if (payload_size <= AXIOM_RAW_PAYLOAD_MAX_SIZE) {
        ret = axiom_send_raw(dev, dst_id, port, AXIOM_TYPE_RAW_DATA,
                (axiom_raw_payload_size_t)(payload_size), payload);
    } else {
        ret = axiom_send_long(dev, dst_id, port,
                (axiom_long_payload_size_t)(payload_size), payload);
    }

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));

    return ret;
}

axiom_err_t
axiom_send_iov(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        size_t payload_size, struct iovec *iov, int iovcnt)
{
    axiom_err_t ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_SEND_IOV));

    if (payload_size <= AXIOM_RAW_PAYLOAD_MAX_SIZE) {
        ret = axiom_send_iov_raw(dev, dst_id, port, AXIOM_TYPE_RAW_DATA,
                (axiom_raw_payload_size_t)(payload_size), iov, iovcnt);
    } else {
        ret = axiom_send_iov_long(dev, dst_id, port,
                (axiom_long_payload_size_t)(payload_size), iov, iovcnt);
    }

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));

    return ret;
}

axiom_err_t
axiom_recv(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, size_t *payload_size, void *payload)
{
    axiom_err_t ret;
    int timeout = -1, fds_ready, i;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RECV));

    if (unlikely(!dev)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (dev->flags & (AXIOM_FLAG_NOBLOCK_RAW | AXIOM_FLAG_NOBLOCK_LONG))
        timeout = 0;

    fds_ready = poll(dev->recv_fds, AXIOM_NUM_RECV_FDS, timeout);

    if (fds_ready < 0) {
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (fds_ready == 0) {
        ret = AXIOM_RET_NOTAVAIL;
        goto end;
    }

    for (i = 0; i < AXIOM_NUM_RECV_FDS; i++) {
        ret = AXIOM_RET_ERROR;
        if (dev->recv_fds[i].revents & POLLIN) {
            if (dev->recv_fds[i].fd == dev->fd_raw) {
                axiom_raw_payload_size_t raw_psize = AXIOM_RAW_PAYLOAD_MAX_SIZE;
                if (*payload_size < AXIOM_RAW_PAYLOAD_MAX_SIZE) {
                    raw_psize = (axiom_raw_payload_size_t)(*payload_size);
                }
                ret = axiom_recv_raw(dev, src_id, port, type, &raw_psize,
                        payload);
                *payload_size = raw_psize;
                break;
            } else if (dev->recv_fds[i].fd == dev->fd_long) {
                axiom_long_payload_size_t long_psize =
                    AXIOM_LONG_PAYLOAD_MAX_SIZE;
                if (*payload_size < AXIOM_LONG_PAYLOAD_MAX_SIZE) {
                    long_psize = (axiom_long_payload_size_t)(*payload_size);
                }
                ret = axiom_recv_long(dev, src_id, port, &long_psize, payload);
                *type = AXIOM_TYPE_LONG_DATA;
                *payload_size = long_psize;
                break;
            }
        }
    }

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_recv_iov(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_type_t *type, size_t *payload_size, struct iovec *iov, int iovcnt)
{
    axiom_err_t ret;
    int timeout = -1, fds_ready, i;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RECV_IOV));

    if (unlikely(!dev)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (dev->flags & (AXIOM_FLAG_NOBLOCK_RAW | AXIOM_FLAG_NOBLOCK_LONG))
        timeout = 0;

    fds_ready = poll(dev->recv_fds, AXIOM_NUM_RECV_FDS, timeout);

    if (fds_ready < 0) {
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (fds_ready == 0) {
        ret = AXIOM_RET_NOTAVAIL;
        goto end;
    }

    for (i = 0; i < AXIOM_NUM_RECV_FDS; i++) {
        ret = AXIOM_RET_ERROR;
        if (dev->recv_fds[i].revents & POLLIN) {
            if (dev->recv_fds[i].fd == dev->fd_raw) {
                axiom_raw_payload_size_t raw_psize = AXIOM_RAW_PAYLOAD_MAX_SIZE;
                if (*payload_size < AXIOM_RAW_PAYLOAD_MAX_SIZE) {
                    raw_psize = (axiom_raw_payload_size_t)(*payload_size);
                }
                ret = axiom_recv_iov_raw(dev, src_id, port, type, &raw_psize,
                        iov, iovcnt);
                *payload_size = raw_psize;
                break;
            } else if (dev->recv_fds[i].fd == dev->fd_long) {
                axiom_long_payload_size_t long_psize =
                    AXIOM_LONG_PAYLOAD_MAX_SIZE;
                if (*payload_size < AXIOM_LONG_PAYLOAD_MAX_SIZE) {
                    long_psize = (axiom_long_payload_size_t)(*payload_size);
                }
                ret = axiom_recv_iov_long(dev, src_id, port, &long_psize,
                        iov, iovcnt);
                *type = AXIOM_TYPE_LONG_DATA;
                *payload_size = long_psize;
                break;
            }
        }
    }

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

inline static axiom_err_t
axiom_send_raw_prepare(axiom_raw_hdr_t *header, axiom_node_id_t dst_id,
        axiom_port_t port, axiom_type_t type,
        axiom_raw_payload_size_t payload_size)
{
    if (unlikely(payload_size > AXIOM_RAW_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    header->tx.port_type.field.port = (port & 0x7);
    header->tx.port_type.field.type = (type & 0x7);
    header->tx.dst = dst_id;
    header->tx.payload_size = payload_size;

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_raw_payload_size_t payload_size,
        void *payload)
{
    axiom_ioctl_raw_t raw_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_SEND_RAW));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = axiom_send_raw_prepare(&raw_msg.header, dst_id, port, type,
            payload_size);
    if (unlikely(!AXIOM_RET_IS_OK(ret)))
        goto end;

    raw_msg.payload = payload;

    ret = ioctl(dev->fd_raw, AXNET_SEND_RAW, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    DPRINTF("dst: 0x%x port: %d payload_size: 0x%x", raw_msg.header.tx.dst,
            raw_msg.header.tx.port_type.field.port,
            raw_msg.header.tx.payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_send_iov_raw(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_type_t type, axiom_raw_payload_size_t payload_size,
        struct iovec *iov, int iovcnt)
{
    axiom_ioctl_raw_iov_t raw_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_SEND_IOV_RAW));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = axiom_send_raw_prepare(&raw_msg.header, dst_id, port, type,
            payload_size);
    if (unlikely(!AXIOM_RET_IS_OK(ret)))
        goto end;

    raw_msg.iov = iov;
    raw_msg.iovcnt = iovcnt;

    ret = ioctl(dev->fd_raw, AXNET_SEND_RAW_IOV, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    DPRINTF("dst: 0x%x port: %d payload_size: 0x%x", raw_msg.header.tx.dst,
            raw_msg.header.tx.port_type.field.port,
            raw_msg.header.tx.payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

inline static axiom_err_t
axiom_recv_raw_finalize(axiom_raw_hdr_t *header, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_type_t *type,
        axiom_raw_payload_size_t *payload_size)
{

    *src_id = header->rx.src;
    *port = (header->rx.port_type.field.port & 0x7);
    *type = (header->rx.port_type.field.type & 0x7);
    *payload_size = header->rx.payload_size;

    DPRINTF("src: 0x%x port: %d payload_size: 0x%x", header->rx.src,
            header->rx.port_type.field.port,
            header->rx.payload_size);

    return header->rx.msg_id;
}

axiom_err_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_type_t *type,
        axiom_raw_payload_size_t *payload_size, void *payload)
{
    axiom_ioctl_raw_t raw_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RECV_RAW));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(*payload_size > AXIOM_RAW_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    raw_msg.header.rx.payload_size = *payload_size;
    raw_msg.payload = payload;

    ret = ioctl(dev->fd_raw, AXNET_RECV_RAW, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    ret = axiom_recv_raw_finalize(&raw_msg.header, src_id, port, type,
            payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_recv_iov_raw(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_type_t *type,
        axiom_raw_payload_size_t *payload_size, struct iovec *iov, int iovcnt)
{
    axiom_ioctl_raw_iov_t raw_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_RECV_IOV_RAW));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(*payload_size > AXIOM_RAW_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    raw_msg.header.rx.payload_size = *payload_size;

    raw_msg.iov = iov;
    raw_msg.iovcnt = iovcnt;

    ret = ioctl(dev->fd_raw, AXNET_RECV_RAW_IOV, &raw_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    ret = axiom_recv_raw_finalize(&raw_msg.header, src_id, port, type,
            payload_size);
end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

int
axiom_send_raw_avail(axiom_dev_t *dev)
{
    int ret, avail;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_SEND_RAW_AVAIL));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = ioctl(dev->fd_raw, AXNET_SEND_RAW_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = avail;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

int
axiom_recv_raw_avail(axiom_dev_t *dev)
{
    int ret, avail;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_RECV_RAW_AVAIL));

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = ioctl(dev->fd_raw, AXNET_RECV_RAW_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = avail;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_flush_raw(axiom_dev_t *dev)
{
    int ret;

    if (unlikely(!dev || dev->fd_raw <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_raw, AXNET_FLUSH_RAW);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

inline static axiom_err_t
axiom_send_long_prepare(axiom_rdma_hdr_t *header, axiom_node_id_t dst_id,
        axiom_port_t port, axiom_long_payload_size_t payload_size)
{
    if (unlikely(payload_size > AXIOM_LONG_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", payload_size,
                AXIOM_LONG_PAYLOAD_MAX_SIZE);
        return AXIOM_RET_ERROR;
    }

    header->tx.port_type.field.port = (port & 0x7);
    header->tx.port_type.field.type = (AXIOM_TYPE_LONG_DATA & 0x7);
    header->tx.dst = dst_id;
    header->tx.payload_size = payload_size;

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_send_long(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_long_payload_size_t payload_size, void *payload)
{
    axiom_long_msg_t long_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_SEND_LONG));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = axiom_send_long_prepare(&long_msg.header, dst_id, port,
            payload_size);
    if (unlikely(!AXIOM_RET_IS_OK(ret)))
        goto end;

    long_msg.payload = payload;

    ret = ioctl(dev->fd_long, AXNET_SEND_LONG, &long_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    DPRINTF("dst: 0x%x payload_size: 0x%x", long_msg.header.tx.dst,
            long_msg.header.tx.payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_send_iov_long(axiom_dev_t *dev, axiom_node_id_t dst_id, axiom_port_t port,
        axiom_long_payload_size_t payload_size, struct iovec *iov, int iovcnt)
{
    axiom_ioctl_long_iov_t long_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_SEND_IOV_LONG));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = axiom_send_long_prepare(&long_msg.header, dst_id, port,
            payload_size);
    if (unlikely(!AXIOM_RET_IS_OK(ret)))
        goto end;

    long_msg.iov = iov;
    long_msg.iovcnt = iovcnt;

    ret = ioctl(dev->fd_long, AXNET_SEND_LONG_IOV, &long_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    DPRINTF("dst: 0x%x payload_size: 0x%x", long_msg.header.tx.dst,
            long_msg.header.tx.payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

inline static axiom_err_t
axiom_recv_long_finalize(axiom_rdma_hdr_t *header, axiom_node_id_t *src_id,
        axiom_port_t *port, axiom_long_payload_size_t *payload_size)
{
    *src_id = header->rx.src;
    *port = (header->rx.port_type.field.port & 0x7);
    *payload_size = header->rx.payload_size;

    return header->rx.msg_id;
}

axiom_err_t
axiom_recv_long(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_long_payload_size_t *payload_size, void *payload)
{
    axiom_long_msg_t long_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RECV_LONG));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(*payload_size > AXIOM_LONG_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_LONG_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    long_msg.header.rx.payload_size = *payload_size;
    long_msg.payload = payload;

    ret = ioctl(dev->fd_long, AXNET_RECV_LONG, &long_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    ret = axiom_recv_long_finalize(&long_msg.header, src_id, port,
            payload_size);

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_recv_iov_long(axiom_dev_t *dev, axiom_node_id_t *src_id, axiom_port_t *port,
        axiom_long_payload_size_t *payload_size, struct iovec *iov, int iovcnt)
{
    axiom_ioctl_long_iov_t long_msg;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_RECV_IOV_LONG));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(*payload_size > AXIOM_LONG_PAYLOAD_MAX_SIZE)) {
        EPRINTF("payload size too big - size: %d [%d]", *payload_size,
                AXIOM_LONG_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    long_msg.header.rx.payload_size = *payload_size;

    long_msg.iov = iov;
    long_msg.iovcnt = iovcnt;

    ret = ioctl(dev->fd_long, AXNET_RECV_LONG_IOV, &long_msg);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    ret = axiom_recv_long_finalize(&long_msg.header, src_id, port,
            payload_size);
end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

int
axiom_send_long_avail(axiom_dev_t *dev)
{
    int ret, avail;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_SEND_LONG_AVAIL));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = ioctl(dev->fd_long, AXNET_SEND_LONG_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = avail;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

int
axiom_recv_long_avail(axiom_dev_t *dev)
{
    int ret, avail;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic,
                AX_EXTRAE_APINIC_RECV_LONG_AVAIL));

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = ioctl(dev->fd_long, AXNET_RECV_LONG_AVAIL, &avail);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    ret = avail;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_flush_long(axiom_dev_t *dev)
{
    int ret;

    if (unlikely(!dev || dev->fd_long <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_long, AXNET_FLUSH_LONG);

    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

#define AXIOM_RDMA_FIXED_ADDR           ((void *)(0x3000000000))
void *
axiom_rdma_mmap(axiom_dev_t *dev, size_t *sizep)
{
    uint64_t size = *sizep;
    void *addr;
    int ret;

    if (unlikely(!dev || dev->fd_rdma <= 0 || !sizep)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return NULL;
    }

    if (unlikely(dev->rdma_addr)) {
        EPRINTF("axiom rdma zone already mapped");
        return NULL;
    }

    ret = ioctl(dev->fd_rdma, AXNET_RDMA_SIZE, &size);
    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return NULL;
    }

    if (unlikely(size <= 0)) {
        EPRINTF("size error [%" PRIu64 "]", size);
        return NULL;
    }

    addr = mmap(AXIOM_RDMA_FIXED_ADDR, size, PROT_READ | PROT_WRITE,
            MAP_SHARED, dev->fd_rdma, 0);
    if (unlikely(addr == MAP_FAILED)) {
        EPRINTF("mmap failed - addr: %p - errno: %s", addr, strerror(errno));
        return NULL;
    }

    if (unlikely(addr != AXIOM_RDMA_FIXED_ADDR)) {
        EPRINTF("mmap failed - addr: %p - errno: %s", addr, strerror(errno));
        axiom_rdma_munmap(dev);
        return NULL;
    }

    dev->rdma_addr = addr;
    dev->rdma_size = size;
    *sizep = size;

    return addr;
}

axiom_err_t
axiom_rdma_munmap(axiom_dev_t *dev)
{
    int ret;

    if (unlikely(!dev || dev->fd_rdma <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    if (unlikely(!dev->rdma_addr)) {
        EPRINTF("axiom rdma zone not mapped");
        return AXIOM_RET_ERROR;
    }

    ret = munmap(dev->rdma_addr, dev->rdma_size);
    if (unlikely(ret)) {
        EPRINTF("munmap failed - errno: %s", strerror(errno));
        return AXIOM_RET_ERROR;
    }

    dev->rdma_addr = NULL;
    dev->rdma_size = 0;

    return AXIOM_RET_OK;
}

static axiom_err_t
axiom_rdma_write_internal(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *local_src_addr, void *remote_dst_addr,
        axiom_token_t *token, uint32_t flags)
{
    axiom_ioctl_rdma_t rdma;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RDMA_WRITE));

    if (unlikely(!dev || dev->fd_rdma <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(payload_size & ((1 << AXIOM_RDMA_PAYLOAD_SIZE_ORDER) - 1))) {
        EPRINTF("payload size [%zu] must be multiple of %d", payload_size,
                (1 << AXIOM_RDMA_PAYLOAD_SIZE_ORDER));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(payload_size > (AXIOM_RDMA_PAYLOAD_MAX_SIZE))) {
        EPRINTF("payload size too big - size: %zu [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    DPRINTF("[packet] payload_size: 0x%x", payload_size);

    rdma.header.tx.port_type.field.type = AXIOM_TYPE_RDMA_WRITE;
    rdma.header.tx.port_type.field.s = 0;
    rdma.header.tx.dst = remote_id;
    rdma.header.tx.payload_size = payload_size;

    rdma.app_id = dev->appid;
    rdma.src_addr = local_src_addr;
    rdma.dst_addr = remote_dst_addr;
    rdma.flags = flags;

    ret = ioctl(dev->fd_rdma, AXNET_RDMA_WRITE, &rdma);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    if (token) {
        *token = rdma.token;
    }

    ret = rdma.token.rdma.msg_id;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_rdma_write_sync(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *local_src_addr, void *remote_dst_addr,
        axiom_token_t *token)
{
    return axiom_rdma_write_internal(dev, remote_id, payload_size,
            local_src_addr, remote_dst_addr, token, 0);
}

axiom_err_t
axiom_rdma_write(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *local_src_addr, void *remote_dst_addr,
        axiom_token_t *token)
{
    return axiom_rdma_write_internal(dev, remote_id, payload_size,
            local_src_addr, remote_dst_addr, token, AXIOCTL_RDMA_FLAGS_ASYNC);
}

static axiom_err_t
axiom_rdma_read_internal(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *remote_src_addr, void *local_dst_addr,
        axiom_token_t *token, uint32_t flags)
{
    axiom_ioctl_rdma_t rdma;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RDMA_READ));

    if (unlikely(!dev || dev->fd_rdma <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(payload_size & ((1 << AXIOM_RDMA_PAYLOAD_SIZE_ORDER) - 1))) {
        EPRINTF("payload size [%zu] must be multiple of %d", payload_size,
                (1 << AXIOM_RDMA_PAYLOAD_SIZE_ORDER));
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    if (unlikely(payload_size > (AXIOM_RDMA_PAYLOAD_MAX_SIZE))) {
        EPRINTF("payload size too big - size: %zu [%d]", payload_size,
                AXIOM_RAW_PAYLOAD_MAX_SIZE);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    rdma.header.tx.port_type.field.type = AXIOM_TYPE_RDMA_READ;
    rdma.header.tx.port_type.field.s = 0;
    rdma.header.tx.dst = remote_id;
    rdma.header.tx.payload_size = payload_size;

    rdma.app_id = dev->appid;
    rdma.src_addr = remote_src_addr;
    rdma.dst_addr = local_dst_addr;
    rdma.flags = flags;

    ret = ioctl(dev->fd_rdma, AXNET_RDMA_READ, &rdma);
    if (unlikely(ret < 0)) {
        if (errno == EAGAIN) {
            ret = AXIOM_RET_NOTAVAIL;
        } else if (errno == EINTR) {
            ret = AXIOM_RET_INTR;
        } else {
            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
        }
        goto end;
    }

    if (token) {
        *token = rdma.token;
    }

    ret = rdma.token.rdma.msg_id;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

axiom_err_t
axiom_rdma_read_sync(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *remote_src_addr, void *local_dst_addr,
        axiom_token_t *token)
{
    return axiom_rdma_read_internal(dev, remote_id, payload_size,
            remote_src_addr, local_dst_addr, token, 0);
}

axiom_err_t
axiom_rdma_read(axiom_dev_t *dev, axiom_node_id_t remote_id,
        size_t payload_size, void *remote_src_addr, void *local_dst_addr,
        axiom_token_t *token)
{
    return axiom_rdma_read_internal(dev, remote_id, payload_size,
            remote_src_addr, local_dst_addr, token, AXIOCTL_RDMA_FLAGS_ASYNC);
}

axiom_err_t
axiom_rdma_check(axiom_dev_t *dev, axiom_token_t *tokens, int tokencnt)
{
    axiom_ioctl_token_t token_ioctl;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RDMA_CHECK));
    if (unlikely(!dev || dev->fd_rdma <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    token_ioctl.tokens = tokens;
    token_ioctl.count = tokencnt;

    ret = ioctl(dev->fd_rdma, AXNET_RDMA_CHECK, &token_ioctl);
    if (unlikely(ret < 0)) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        ret = AXIOM_RET_ERROR;
    }

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

static axiom_err_t
axiom_rdma_wait_internal(axiom_dev_t *dev, axiom_token_t *token)
{
    axiom_ioctl_token_t token_ioctl;

    token_ioctl.tokens = token;
    token_ioctl.count = 1;

    return ioctl(dev->fd_rdma, AXNET_RDMA_WAIT, &token_ioctl);
}

axiom_err_t
axiom_rdma_wait(axiom_dev_t *dev, axiom_token_t *tokens, int tokencnt)
{
    int acked;
    int ret;

    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_RDMA_WAIT));

    if (unlikely(!dev || dev->fd_rdma <= 0)) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        ret = AXIOM_RET_ERROR;
        goto end;
    }

    acked = axiom_rdma_check(dev, tokens, tokencnt);
    if (!AXIOM_RET_IS_OK(acked)) {
        return acked;
    }

    while (acked < tokencnt) {
        int tkn_id;

        for (tkn_id = tokencnt - 1; tkn_id >= 0; tkn_id--) {
            if (tokens[tkn_id].rdma.status == AXIOM_TOKEN_PENDING)
                break;
        }

        ret = axiom_rdma_wait_internal(dev, &tokens[tkn_id]);
        if (unlikely(ret < 0)) {
            if (errno == EAGAIN) {
                ret = AXIOM_RET_NOTAVAIL;
                break;
            }

            EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
            ret = AXIOM_RET_ERROR;
            break;
        }

        acked = axiom_rdma_check(dev, tokens, tokencnt);
        if (!AXIOM_RET_IS_OK(acked)) {
            return acked;
        }
    }

    ret = AXIOM_RET_OK;

end:
    AXIOM_EXTRAE(Extrae_event(axiom_extrae_apinic, AX_EXTRAE_APINIC_END));
    return ret;
}

uint32_t
axiom_read_ni_status(axiom_dev_t *dev)
{
    int ret;
    uint32_t status;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_GET_STATUS, &status);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return status;
}

void
axiom_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    int ret;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return;
    }

    ret = ioctl(dev->fd_generic, AXNET_SET_CONTROL, &reg_mask);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }
}

uint32_t
axiom_read_ni_control(axiom_dev_t *dev)
{
    int ret;
    uint32_t control;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_GET_CONTROL, &control);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return control;
}


void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    int ret;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return;
    }

    ret = ioctl(dev->fd_generic, AXNET_SET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }
}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    int ret;
    axiom_node_id_t node_id;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_GET_NODEID, &node_id);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
    }

    return node_id;
}

axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = enabled_mask;

    ret = ioctl(dev->fd_generic, AXNET_SET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    int ret;
    axiom_ioctl_routing_t routing;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    routing.node_id = node_id;
    routing.enabled_mask = 0;

    ret = ioctl(dev->fd_generic, AXNET_GET_ROUTING, &routing);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    *enabled_mask = routing.enabled_mask;

    return AXIOM_RET_OK;
}

int
axiom_get_num_nodes(axiom_dev_t *dev)
{
    axiom_node_id_t i, node_id;
    uint8_t enabled_mask;
    axiom_err_t err;
    /* init to 1, because the local node is not set in the routing table */
    int num_nodes = 1;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    /* get local id */
    node_id = axiom_get_node_id(dev);

    for (i = 0; i < AXIOM_NODES_MAX; i++) {

        /* we already count local node */
        if (i == node_id)
            continue;

        err = axiom_get_routing(dev, i, &enabled_mask);
        if (err)
            break;

        /*
         * count node i, if it is reachable through physical interfaces
         * discard nodes reachable through the loopback interface (IF0)
         */
        if (enabled_mask != 0x0 && enabled_mask != 0x1)
            num_nodes++;
    }

    return num_nodes;
}


axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    int ret;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_GET_IFNUMBER, if_number);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    int ret;
    uint8_t buf_if = if_number;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_GET_IFINFO, &buf_if);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    *if_features = buf_if;

    return AXIOM_RET_OK;
}


axiom_err_t
axiom_debug_info(axiom_dev_t *dev)
{
    int ret;

    if (!dev || dev->fd_generic <= 0) {
        EPRINTF("axiom device is not opened - dev: %p", dev);
        return AXIOM_RET_ERROR;
    }

    ret = ioctl(dev->fd_generic, AXNET_DEBUG_INFO);

    if (ret < 0) {
        EPRINTF("ioctl error - ret: %d errno: %s", ret, strerror(errno));
        return AXIOM_RET_ERROR;
    }

    return AXIOM_RET_OK;
}
