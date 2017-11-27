/*!
 * \file axiom_netdev_module.c
 *
 * \version     v0.15
 * \date        2016-05-03
 *
 * This file contains the implementation of the Axiom NIC kernel module.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include "axiom_netdev.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence SRL");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("v0.15");

/*! \brief verbose module parameter */
int verbose = 0;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "versbose level (0=none,...,16=all)");

/*! \brief delay (usec) module parameter */
int retry_udelay = 200;
module_param(retry_udelay, int, 0644);
MODULE_PARM_DESC(retry_udelay, "usec to sleep when retry to TX a long packet");

/*! \brief period (msec) of watchdog */
int watchdog_period = 100;
module_param(watchdog_period, int, 0644);
MODULE_PARM_DESC(watchdog_period, "watchdog period in msec");

struct axiomnet_chrdev chrdev;

static int axiomnet_alloc_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);
static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev);

#define AXIOM_CACHE_WORKAROUND
#ifdef AXIOM_CACHE_WORKAROUND
/*
 * With no-cached memory regions the memcpy() generates a fault if the buffers
 * are not aligned to 8 bytes.
 * This is a workaround, waiting a FORTH bitstream that support RDMA cached
 * memory.
 */
#define AXIOM_RDMA_VADDR       (0x4000000000)
#define AXIOM_RDMA_SIZE        (1024 * 1024 * 1024)
#define AXIOM_COPY_ALIGN       (8UL)
#define AXIOM_COPY_MASK        (AXIOM_COPY_ALIGN - 1)
static inline unsigned long axiom_copy_from_user(void *to,
        const void __user *from, unsigned long n)
{
    unsigned long offset = 0, ret = 0, odd_to, odd_from;
    uint8_t *actual_to = (uint8_t *) to;
    uint8_t *actual_from = (uint8_t *) from;

    if (!access_ok(VERIFY_READ, from, n)) {
        EPRINTF("access_ok failed");
        return n;
    }

#if 0
    if (actual_from < AXIOM_RDMA_VADDR ||
            actual_from >= (AXIOM_RDMA_VADDR + AXIOM_RDMA_SIZE)) {
        return __copy_from_user(to, from, n);
    }
#endif

    odd_to = AXIOM_COPY_ALIGN - (((uintptr_t)actual_to) & AXIOM_COPY_MASK);
    odd_from = AXIOM_COPY_ALIGN - (((uintptr_t)actual_from) & AXIOM_COPY_MASK);

    if (odd_to == AXIOM_COPY_ALIGN && odd_from == AXIOM_COPY_ALIGN) {
        ret = __copy_from_user(to, from, n);
    } else {
        if (odd_to != odd_from) {
            while (offset < n) {
                __get_user(*(actual_to++), actual_from++);
                ++offset;
            }
        } else {
            while (offset < odd_to) {
                __get_user(*(actual_to++), actual_from++);
                ++offset;
            }

            ret = __copy_from_user(actual_to, actual_from, n - offset);
        }
    }

    return ret;
}

static inline unsigned long axiom_copy_to_user(void __user *to,
        const void *from, unsigned long n)
{
    unsigned long offset = 0, ret = 0, odd_to, odd_from;
    uint8_t *actual_to = (uint8_t *) to;
    uint8_t *actual_from = (uint8_t *) from;

    if (!access_ok(VERIFY_WRITE, to, n)) {
        EPRINTF("access_ok failed");
        return n;
    }

#if 0
    if (actual_to < AXIOM_RDMA_VADDR ||
            actual_to >= (AXIOM_RDMA_VADDR + AXIOM_RDMA_SIZE)) {
        return __copy_to_user(to, from, n);
    }
#endif

    odd_to = AXIOM_COPY_ALIGN - (((uintptr_t)actual_to) & AXIOM_COPY_MASK);
    odd_from = AXIOM_COPY_ALIGN - (((uintptr_t)actual_from) & AXIOM_COPY_MASK);

    if (odd_to == AXIOM_COPY_ALIGN && odd_from == AXIOM_COPY_ALIGN) {
        ret = __copy_to_user(to, from, n);
    } else {
        if (odd_to != odd_from) {
            while (offset < n) {
                __put_user(*(actual_from++), actual_to++);
                ++offset;
            }
        } else {
            while (offset < odd_to) {
                __put_user(*(actual_from++), actual_to++);
                ++offset;
            }

            ret = __copy_to_user(actual_to, actual_from, n - offset);
        }
    }

    return ret;
}
#else /* !AXIOM_CACHE_WORKAROUND */
#define axiom_copy_from_user		copy_from_user
#define axiom_copy_to_user		copy_to_user
#endif /* AXIOM_CACHE_WORKAROUND */

/************************ AxiomNet Device Driver ******************************/

void axiomnet_irqhandler(struct axiomnet_drvdata *drvdata)
{
    uint32_t irq_pending;

    DPRINTF("start");
    irq_pending = axiom_hw_pending_irq(drvdata->dev_api);

    if (irq_pending & AXIOMREG_IRQ_RAW_RX) {
        axiom_kthread_wakeup(&drvdata->kthread_raw);
        drvdata->stats.irq_raw_rx++;
    }

    if (irq_pending & AXIOMREG_IRQ_RAW_TX) {
        wake_up(&(drvdata->raw_tx_ring.port.wait_queue));
        drvdata->stats.irq_raw_tx++;
    }

    if (irq_pending & AXIOMREG_IRQ_RDMA_RX) {
        axiom_kthread_wakeup(&drvdata->kthread_rdma);
        drvdata->stats.irq_rdma_rx++;
    }

    if (irq_pending & AXIOMREG_IRQ_RDMA_TX) {
        wake_up(&(drvdata->rdma_tx_ring.rdma_port.wait_queue));
        drvdata->stats.irq_rdma_tx++;
    }

    drvdata->stats.irq++;

    axiom_hw_ack_irq(drvdata->dev_api, irq_pending);

    DPRINTF("end");
}

/****************************** RAW functions *********************************/

inline static int axiomnet_raw_tx_avail(struct axiomnet_raw_tx_hwring *tx_ring)
{
    /*TODO: try to minimize the register read */
    return axiom_hw_raw_tx_avail(tx_ring->drvdata->dev_api);
}

inline static int axiomnet_raw_rx_avail(struct axiomnet_raw_rx_hwring *rx_ring,
        int port)
{
    int avail;

    avail = eviq_avail(&rx_ring->sw_queue.evi_queue, port);
    DPRINTF("queue - avail %d port: %d", avail, port);

    return avail;
}

inline static int axiomnet_raw_send(struct file *filep,
        axiom_raw_hdr_t *header, const struct iovec *iov, int iovcnt)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_tx_hwring *tx_ring = &drvdata->raw_tx_ring;
    axiom_raw_msg_t raw_msg;
    int ret, i, offset;

    DPRINTF("start");

    if (unlikely(header->tx.payload_size > sizeof(raw_msg.payload))) {
        return -EFBIG;
    }

    if (unlikely(header->tx.port_type.field.type != AXIOM_TYPE_RAW_NEIGHBOUR &&
                drvdata->routing_table[header->tx.dst] == 0x0)) {
        return -ENXIO;
    }

    mutex_lock(&tx_ring->port.mutex);

    while (axiomnet_raw_tx_avail(tx_ring) == 0) { /* no space to write */
        drvdata->stats.wait_raw_tx++;
        mutex_unlock(&tx_ring->port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(tx_ring->port.wait_queue,
                    axiomnet_raw_tx_avail(tx_ring) != 0))
            return -ERESTARTSYS;

        mutex_lock(&tx_ring->port.mutex);
    }
    mutex_unlock(&tx_ring->port.mutex);

    offset = 0;
    for (i = 0; i < iovcnt; i++) {
        int copied = iov[i].iov_len;

        if ((copied + offset) > header->tx.payload_size) {
            ret = -EFBIG;
            EPRINTF("iov[%d] - iovcnt: %d psize: %d offset: %d copied: %d",
                    i, iovcnt, header->tx.payload_size, offset, copied);
            goto err;
        }

        ret = axiom_copy_from_user((uint8_t *)(&(raw_msg.payload)) + offset,
                iov[i].iov_base, copied);
        if (unlikely(ret)) {
            ret = -EFAULT;
            goto err;
        }

        offset += copied;
    }

    memcpy(&(raw_msg.header), header, sizeof(raw_msg.header));

    /* reset error and s bit */
    raw_msg.header.tx.port_type.field.error = 0;
    raw_msg.header.tx.port_type.field.s = 0;

    mutex_lock(&tx_ring->port.mutex);
    /* copy packet into the ring */
    ret = axiom_hw_raw_tx(tx_ring->drvdata->dev_api, &(raw_msg));
    if (unlikely(ret < 0)) {
        drvdata->stats.err_raw_tx++;
        ret = -EFAULT;
        goto err;
    }

    drvdata->stats.pkt_raw_tx++;
    drvdata->stats.bytes_raw_tx += header->tx.payload_size;
    mutex_unlock(&tx_ring->port.mutex);

err:

    DPRINTF("end");

    return ret;
}

inline static bool axiomnet_raw_rx_work_todo(void *data)
{
    struct axiomnet_raw_rx_hwring *rx_ring = data;
    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    unsigned long flags;
    bool ret;

    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    ret = (axiom_hw_raw_rx_avail(rx_ring->drvdata->dev_api) != 0)
        && (eviq_free_avail(&sw_queue->evi_queue) != 0);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    return ret;
}

inline static void axiom_raw_rx_dequeue(struct axiomnet_raw_rx_hwring *rx_ring)
{
    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    struct axiomnet_drvdata *drvdata = rx_ring->drvdata;
    unsigned long flags;
    uint32_t received = 0;
    int port;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    DPRINTF("start");


    /* something to read */
    while (axiomnet_raw_rx_work_todo(rx_ring)) {
        int avail;
        axiom_raw_msg_t *raw_msg;

        spin_lock_irqsave(&sw_queue->queue_lock, flags);
        queue_slot = eviq_free_pop(&sw_queue->evi_queue);
        spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

        if (unlikely(queue_slot == EVIQ_NONE)) {
            EPRINTF("RAW SW queue empty")
            break;
        }

        raw_msg = &(sw_queue->queue_desc[queue_slot]);

        axiom_hw_raw_rx(drvdata->dev_api, raw_msg);
        port = raw_msg->header.rx.port_type.field.port;

        /* check valid port */
        if (unlikely(port < 0 || port > AXIOM_PORT_MAX)) {
            EPRINTF("message discarded - wrong port %d", port);

            drvdata->stats.err_raw_rx++;

            spin_lock_irqsave(&sw_queue->queue_lock, flags);
            eviq_free_push(&sw_queue->evi_queue, queue_slot);
            spin_unlock_irqrestore(&sw_queue->queue_lock, flags);
            continue;
        }

        spin_lock_irqsave(&sw_queue->queue_lock, flags);
        avail = eviq_avail(&sw_queue->evi_queue, port);
        eviq_enqueue(&sw_queue->evi_queue, port, queue_slot);
        spin_unlock_irqrestore(&sw_queue->queue_lock, flags);
        if (avail == 0)
            wake_up(&rx_ring->ports[port].wait_queue);

        DPRINTF("queue insert - received: %d queue_slot: %d port: %d", received,
                queue_slot, port);

        received++;

        drvdata->stats.pkt_raw_rx++;
        drvdata->stats.bytes_raw_rx += raw_msg->header.tx.payload_size;
    }

    DPRINTF("received: %d", received);

    DPRINTF("end");
}

inline static ssize_t axiomnet_raw_recv(struct file *filep,
        axiom_raw_hdr_t *header, const struct iovec *iov, int iovcnt)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    int port = priv->bind_port, ret;
    int i, offset, avail;
    ssize_t len;

    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    axiom_raw_msg_t *raw_msg;
    eviq_pnt_t queue_slot;
    unsigned long flags;

    DPRINTF("start");

    /* check bind */
    if (unlikely(port == AXIOMNET_PORT_INVALID)) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    /* we have one mutex per port */
    mutex_lock(&rx_ring->ports[port].mutex);

    while (axiomnet_raw_rx_avail(rx_ring, port) == 0) { /* nothing to read */
        drvdata->stats.wait_raw_rx++;
        mutex_unlock(&rx_ring->ports[port].mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(rx_ring->ports[port].wait_queue,
                    axiomnet_raw_rx_avail(rx_ring, port) != 0))
            return -ERESTARTSYS;

        mutex_lock(&rx_ring->ports[port].mutex);
    }

    /* copy packet from the ring */
    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    queue_slot = eviq_dequeue(&sw_queue->evi_queue, port);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->ports[port].mutex);

    /* XXX: impossible! */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        len = -EFAULT;
        goto err;
    }
    DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);

    raw_msg = &(sw_queue->queue_desc[queue_slot]);

    if (unlikely(header->rx.payload_size < raw_msg->header.rx.payload_size)) {
        EPRINTF("payload received too big - payload: available %d - received %d",
                header->rx.payload_size, raw_msg->header.rx.payload_size);
        len = -EFBIG;
        goto free_enqueue;
    }

    memcpy(header, &(raw_msg->header), sizeof(*header));

    offset = 0;
    for (i = 0; (i < iovcnt) &&
            (offset < raw_msg->header.rx.payload_size); i++) {
        int copied = min((int)(iov[i].iov_len),
                (int)(raw_msg->header.rx.payload_size - offset));

        ret = axiom_copy_to_user(iov[i].iov_base,
                (uint8_t *)(&(raw_msg->payload)) + offset, copied);
        if (unlikely(ret)) {
            len = -EFAULT;
            goto free_enqueue;
        }

        offset += copied;
    }

    len = sizeof(*header) + raw_msg->header.rx.payload_size;

free_enqueue:
    spin_lock_irqsave(&sw_queue->queue_lock, flags);
    avail = eviq_free_avail(&sw_queue->evi_queue);
    eviq_free_push(&sw_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);
    /* send a notification to kthread */
    if (avail == 0)
        axiom_kthread_wakeup(&drvdata->kthread_raw);

err:
    DPRINTF("end len:%zu", len);
    return len;
}

static long axiomnet_raw_flush(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    struct axiomnet_raw_queue *sw_queue = &rx_ring->sw_queue;
    int port = priv->bind_port;
    unsigned long flags;
    long ret = 0;

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    mutex_lock(&rx_ring->ports[port].mutex);

    /* take the lock to avoid enqueue during the flush */
    spin_lock_irqsave(&sw_queue->queue_lock, flags);

    while (axiomnet_raw_rx_avail(rx_ring, port) != 0) {
        eviq_pnt_t queue_slot;

        queue_slot = eviq_dequeue(&sw_queue->evi_queue, port);

        /* XXX: impossible! */
        if (queue_slot == EVIQ_NONE) {
            ret = -EFAULT;
            goto err;
        }

        eviq_free_push(&sw_queue->evi_queue, queue_slot);

        DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);
    }

err:
    spin_unlock_irqrestore(&sw_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->ports[port].mutex);

    /* send a notification to kthread */
    axiom_kthread_wakeup(&drvdata->kthread_raw);

    return ret;
}

inline static struct axiomnet_long_buf_lut *
axiomnet_long_rdma2buf(struct axiomnet_drvdata *drvdata,
        axiom_addr_t rdma_addr)
{
    int buf_id = (rdma_addr - drvdata->rdma_size) / AXIOM_LONG_PAYLOAD_MAX_SIZE;

    if (unlikely(buf_id >= AXIOMREG_LEN_LONG_BUF || buf_id < 0))
        return NULL;

    return &drvdata->long_rxbuf_lut[buf_id];
}


/***************************** RDMA functions *********************************/

inline static int axiomnet_rdma_tx_avail(struct axiomnet_rdma_tx_hwring *tx_ring)
{
    /*TODO: try to minimize the register read */
    return axiom_hw_rdma_tx_avail(tx_ring->drvdata->dev_api) &&
        eviq_free_avail(&tx_ring->rdma_queue.evi_queue);
}

inline static int axiomnet_rdma_tx(struct file *filep,
        axiom_rdma_hdr_t *header, axiom_token_t *token,
        axiom_callback_t *callback, uint32_t user_flags)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_queue *rdma_queue = &tx_ring->rdma_queue;
    axiom_rdma_status_t *rdma_status;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    unsigned long flags;
    int ret, avail;

    DPRINTF("start");

    if (unlikely(drvdata->routing_table[header->tx.dst] == 0x0)) {
        return -ENXIO;
    }

    mutex_lock(&tx_ring->rdma_port.mutex);

    /* check slot available in the HW ring */
    while (axiomnet_rdma_tx_avail(tx_ring) == 0) { /* no space to write */
        drvdata->stats.wait_rdma_tx++;
        mutex_unlock(&tx_ring->rdma_port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(tx_ring->rdma_port.wait_queue,
                    axiomnet_rdma_tx_avail(tx_ring) != 0))
            return -ERESTARTSYS;

        mutex_lock(&tx_ring->rdma_port.mutex);
    }

    spin_lock_irqsave(&rdma_queue->queue_lock, flags);
    queue_slot = eviq_free_pop(&rdma_queue->evi_queue);
    spin_unlock_irqrestore(&rdma_queue->queue_lock, flags);

    mutex_unlock(&tx_ring->rdma_port.mutex);

    /* impossible */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        ret = -EFAULT;
        return ret;
    }

    rdma_status = &(rdma_queue->queue_desc[queue_slot]);
    header->tx.msg_id = rdma_status->msg_id;

    /* setup token to check the rdma status */
    if (token) {
        token->rdma.msg_id = rdma_status->msg_id;
        token->rdma.status = AXIOM_TOKEN_PENDING;
        token->rdma.value = rdma_status->msg_id_counter;
    }

    /* reset error and s bit */
    header->tx.port_type.field.error = 0;
    header->tx.port_type.field.s = 0;

    rdma_status->ack_received = false;
    rdma_status->queue_slot = queue_slot;
    rdma_status->retries = 0;
    memcpy(&rdma_status->header, header, sizeof(*header));

    if (callback) {
        rdma_status->ack_waiting = false;
        rdma_status->callback = *callback;
    } else {
        /* if it is async call, avoid to wait the ack */
        if (user_flags & AXIOCTL_RDMA_FLAGS_ASYNC) {
            rdma_status->ack_waiting = false;
        } else {
            rdma_status->ack_waiting = true;
        }
        rdma_status->callback.func = NULL;
    }

    mutex_lock(&tx_ring->rdma_port.mutex);
    /* copy packet into the ring */
    ret = axiom_hw_rdma_tx(drvdata->dev_api, header);
    if (unlikely(ret != header->tx.msg_id)) {
        drvdata->stats.err_rdma_tx++;
        ret = -EFAULT;
        goto err;
    }

    drvdata->stats.pkt_rdma_tx++;
    drvdata->stats.bytes_rdma_tx += header->tx.payload_size;

    /* if we don't need to wait, unlock the mutex and return */
    if (!rdma_status->ack_waiting) {
        mutex_unlock(&tx_ring->rdma_port.mutex);
        return ret;
    }

    /* wait the reply */
    while (rdma_status->ack_received == false) {
        drvdata->stats.wait_rdma_rx++;
        mutex_unlock(&tx_ring->rdma_port.mutex);

        /* put the process in the wait_queue to wait the ack */
        if (wait_event_interruptible(rdma_status->wait_queue,
                    rdma_status->ack_received != false)) {
            rdma_status->ack_waiting = false;
            ret = -ERESTARTSYS;
            goto err_nolock;
        }

        mutex_lock(&tx_ring->rdma_port.mutex);
    }

    rdma_status->ack_received = false;
    rdma_status->header.tx.dst = AXIOM_NULL_NODE;

err:
    mutex_unlock(&tx_ring->rdma_port.mutex);

    spin_lock_irqsave(&rdma_queue->queue_lock, flags);
    avail = eviq_free_avail(&rdma_queue->evi_queue);
    eviq_free_push(&rdma_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&rdma_queue->queue_lock, flags);
    /* send a notification to other thread */
    if (avail == 0)
        wake_up(&(tx_ring->rdma_port.wait_queue));

err_nolock:
    DPRINTF("end");

    return ret;
}

static long axiomnet_rdma_check(struct file *filep,
        axiom_ioctl_token_t *token_ioctl)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_queue *rdma_queue = &tx_ring->rdma_queue;
    axiom_token_t *tokens;
    long ret = 0, acked;
    int i;

    tokens = vmalloc(sizeof(*tokens) * token_ioctl->count);
    if (tokens == NULL) {
        return -ENOMEM;
    }

    ret = axiom_copy_from_user(tokens, token_ioctl->tokens,
            sizeof(*tokens) * token_ioctl->count);
    if (ret) {
        ret = -EFAULT;
        goto free_tokens;
    }

    acked = 0;
    for (i = 0; i < token_ioctl->count; i++) {
        axiom_msg_id_t msg_id = tokens[i].rdma.msg_id;
        axiom_rdma_status_t *rdma_status;

        if (tokens[i].rdma.status != AXIOM_TOKEN_PENDING) {
            if (tokens[i].rdma.status == AXIOM_TOKEN_ACKED) {
                acked++;
            }
            continue;
        }

        if (msg_id >= AXIOMNET_RDMA_QUEUE_FREE_LEN) {
            tokens[i].rdma.status = AXIOM_TOKEN_INVALID;
            continue;
        }

        rdma_status = &(rdma_queue->queue_desc[msg_id]);

        if (rdma_status->msg_id_counter != tokens[i].rdma.value) {
            tokens[i].rdma.status = AXIOM_TOKEN_ACKED;
            acked++;
        }
    }

    ret = axiom_copy_to_user(token_ioctl->tokens, tokens,
            sizeof(*tokens) * token_ioctl->count);
    if (ret) {
        ret = -EFAULT;
        goto free_tokens;
    }

    ret = acked;

free_tokens:
    vfree(tokens);
    return ret;
}

static long axiomnet_rdma_wait(struct file *filep,
        axiom_ioctl_token_t *token_ioctl)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_queue *rdma_queue = &tx_ring->rdma_queue;
    axiom_token_t token;
    axiom_msg_id_t msg_id;
    axiom_rdma_status_t *rdma_status;
    long ret;

    if (token_ioctl->count != 1) {
        EPRINTF("Expected only 1 token [token count: %d]", token_ioctl->count);
        return -EINVAL;
    }

    ret = axiom_copy_from_user(&token, token_ioctl->tokens, sizeof(token));
    if (ret) {
        return -EFAULT;
    }

    if (token.rdma.status != AXIOM_TOKEN_PENDING) {
        return 0;
    }

    msg_id = token.rdma.msg_id;
    rdma_status = &(rdma_queue->queue_desc[msg_id]);

    if (msg_id >= AXIOMNET_RDMA_QUEUE_FREE_LEN) {
        EPRINTF("invalid message ID: %d", msg_id);
        return -EINVAL;
    }

    /* wait the reply */
    while (rdma_status->msg_id_counter == token.rdma.value) {
        drvdata->stats.wait_rdma_rx++;
#if 0
        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
#endif
        /* put the process in the wait_queue to wait the ack */
        if (wait_event_interruptible(rdma_status->wait_queue,
                    rdma_status->msg_id_counter != token.rdma.value)) {
            return -ERESTARTSYS;
        }
    }

    token.rdma.status = AXIOM_TOKEN_ACKED;

    ret = axiom_copy_to_user(token_ioctl->tokens, &token, sizeof(token));
    if (ret) {
        return -EFAULT;
    }

    return 0;
}

inline static int axiomnet_long_rx_avail(struct axiomnet_rdma_rx_hwring *rx_ring,
        int port)
{
    int avail;

    avail = eviq_avail(&rx_ring->long_queue.evi_queue, port);
    DPRINTF("queue - avail %d port: %d", avail, port);

    return avail;
}

inline static bool axiomnet_rdma_rx_work_todo(void *data)
{
    struct axiomnet_rdma_rx_hwring *rx_ring = data;
    return (axiom_hw_rdma_rx_avail(rx_ring->drvdata->dev_api) != 0);
}

inline static void axiom_rdma_rx_dequeue(struct axiomnet_rdma_rx_hwring *rx_ring)
{
    struct axiomnet_drvdata *drvdata = rx_ring->drvdata;
    axiom_rdma_hdr_t rdma_hdr;
    axiom_msg_id_t msg_id;

    /* something to read */
    while (axiomnet_rdma_rx_work_todo(rx_ring)) {
        msg_id = axiom_hw_rdma_rx(rx_ring->drvdata->dev_api, &rdma_hdr);

        /* if the s_bit is set, we received an ack, otherwise it is a LONG msg*/
        if (unlikely(rdma_hdr.rx.port_type.field.s == 1)) {
            axiom_rdma_status_t *rdma_status =
                &(rx_ring->tx_rdma_queue->queue_desc[msg_id]);
            struct axiomnet_rdma_queue *rdma_queue = rx_ring->tx_rdma_queue;

            if (unlikely(rdma_status->header.tx.dst != rdma_hdr.rx.src)) {
                EPRINTF("Message (id = %u) discarded - unexpected ACK received "
                        "from %u [expected from %u]", msg_id, rdma_hdr.rx.src,
                        rdma_status->header.tx.dst);

                drvdata->stats.err_rdma_rx++;
                continue;
            }

            /* retry to send packet if there is an error on remote node */
            if (rdma_hdr.rx.port_type.field.error == 1 &&
                    rdma_status->retries < AXIOMNET_MAX_RDMA_RETRY) {
                struct axiomnet_rdma_tx_hwring *tx_ring =
                    &drvdata->rdma_tx_ring;
                int ret;

                mutex_lock(&tx_ring->rdma_port.mutex);

                if (retry_udelay != 0) {
                    /*
                     * sleep with lock acquired, to slow down the senders
                     */
                    usleep_range(retry_udelay, retry_udelay + 10);
                }

                while (!axiom_hw_rdma_tx_avail(drvdata->dev_api)) {
                    /* XXX or wait_event_interruptible? */
                    IPRINTF(1, "sleep 1 msec");
                    msleep(1);
                }

                /* resend the previously packet */
                ret = axiom_hw_rdma_tx(drvdata->dev_api, &rdma_status->header);
                mutex_unlock(&tx_ring->rdma_port.mutex);

                rdma_status->retries++;
                drvdata->stats.retries_rdma++;
                /*
                 * if all is ok, continue to next packet, otherwise free all
                 * resources
                 */
                if (likely(ret == rdma_status->header.tx.msg_id)) {
                    continue;
                }
            }

            if (rdma_hdr.rx.port_type.field.error == 1) {
                EPRINTF("Message discarded after %d retries - "
                        "msg_id: %u dst_id: %u port: %u", rdma_status->retries,
                        rdma_status->header.tx.msg_id,
                        rdma_status->header.tx.dst,
                        rdma_status->header.tx.port_type.field.port);

                drvdata->stats.discarded_rdma++;
            }

            /* if there is some process to wait, wakeup it, otherwise free the
             * status
             */
            rdma_status->msg_id_counter++;
            if (rdma_status->ack_waiting) {
                rdma_status->ack_received = true;
            } else {
                struct axiomnet_rdma_tx_hwring *tx_ring =
                    &rx_ring->drvdata->rdma_tx_ring;
                unsigned long flags;
                int avail;

                if (rdma_status->callback.func) {
                    rdma_status->callback.func(rx_ring->drvdata,
                            rdma_status->callback.data, &rdma_hdr);
                }

                rdma_status->ack_received = false;
                rdma_status->header.tx.dst = AXIOM_NULL_NODE;

                spin_lock_irqsave(&rdma_queue->queue_lock, flags);
                avail = eviq_free_avail(&rdma_queue->evi_queue);
                eviq_free_push(&rdma_queue->evi_queue, rdma_status->queue_slot);
                spin_unlock_irqrestore(&rdma_queue->queue_lock, flags);
                /* send a notification to other thread */
                if (avail)
                    wake_up(&(tx_ring->rdma_port.wait_queue));

            }
            /* wake up waitinig process */
            wake_up(&(rdma_status->wait_queue));

            drvdata->stats.pkt_rdma_rx++;
            drvdata->stats.bytes_rdma_rx += rdma_hdr.rx.payload_size;

        } else { /* LONG message */
            axiom_long_msg_t *long_msg;
            eviq_pnt_t queue_slot = EVIQ_NONE;
            struct axiomnet_long_queue *long_queue = &rx_ring->long_queue;
            unsigned long flags;
            int port, avail;

            if (rdma_hdr.rx.port_type.field.type == AXIOM_TYPE_RDMA_WRITE) {
                drvdata->stats.pkt_rdma_rx++;
                drvdata->stats.bytes_rdma_rx += rdma_hdr.rx.payload_size;
                /*
                 * Actual FORTH implementation, put a descriptor on the
                 * receiver during the RDMA WRITE.
                 * For now we discard it.
                 */
                continue;
            }

            if (unlikely(rdma_hdr.rx.port_type.field.type !=
                        AXIOM_TYPE_LONG_DATA)) {
                EPRINTF("Message discarded - unexpected RDMA received - "
                        "hdr[0]: 0x%llx hdr[1]: 0x%llx",
                        *((uint64_t *)&rdma_hdr),
                        *(((uint64_t *)&rdma_hdr) + 1));

                drvdata->stats.err_rdma_rx++;
                continue;
            }

            spin_lock_irqsave(&long_queue->queue_lock, flags);
            queue_slot = eviq_free_pop(&long_queue->evi_queue);
            spin_unlock_irqrestore(&long_queue->queue_lock, flags);

            if (unlikely(queue_slot == EVIQ_NONE)) {
                EPRINTF("Message discarded - LONG SW queue empty");

                drvdata->stats.err_long_rx++;
                continue;
            }

            long_msg = &(long_queue->queue_desc[queue_slot]);

            /* copy the header in the queue */
            memcpy(&long_msg->header, &rdma_hdr, sizeof(rdma_hdr));

            port = long_msg->header.rx.port_type.field.port;

            /* check valid port */
            if (unlikely(port < 0 || port > AXIOM_PORT_MAX)) {
                EPRINTF("Message discarded - wrong port %d", port);

                drvdata->stats.err_long_rx++;

                spin_lock_irqsave(&long_queue->queue_lock, flags);
                eviq_free_push(&long_queue->evi_queue, queue_slot);
                spin_unlock_irqrestore(&long_queue->queue_lock, flags);
                continue;
            }

            spin_lock_irqsave(&long_queue->queue_lock, flags);
            avail = eviq_avail(&long_queue->evi_queue, port);
            eviq_enqueue(&long_queue->evi_queue, port, queue_slot);
            spin_unlock_irqrestore(&long_queue->queue_lock, flags);
            /* wake up process only when the queue was empty */
            if (avail == 0)
                wake_up(&rx_ring->long_ports[port].wait_queue);

            DPRINTF("queue insert - queue_slot: %d port: %d",
                    queue_slot, port);

            drvdata->stats.pkt_long_rx++;
            drvdata->stats.bytes_long_rx += long_msg->header.rx.payload_size;

        }
    }

}

/***************************** LONG functions *********************************/

static void axiomnet_long_callback(struct axiomnet_drvdata *drvdata, void *data,
        axiom_rdma_hdr_t *rdma_hdr)
{
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_long_queue *long_queue = &tx_ring->long_queue;
    eviq_pnt_t queue_slot = (eviq_pnt_t)(uintptr_t)data;
    unsigned long flags;

    spin_lock_irqsave(&long_queue->queue_lock, flags);
    eviq_free_push(&long_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);
    /* send a notification to other thread */
    wake_up(&(tx_ring->long_port.wait_queue));
}


inline static int axiomnet_long_tx_avail(struct axiomnet_rdma_tx_hwring *tx_ring)
{
    return eviq_free_avail(&tx_ring->long_queue.evi_queue);
}

inline static int axiomnet_long_send(struct file *filep,
        axiom_rdma_hdr_t *user_header, const struct iovec *iov, int iovcnt)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_long_queue *long_queue = &tx_ring->long_queue;
    axiom_callback_t cb;
    axiom_long_msg_t *long_msg;
    eviq_pnt_t queue_slot = EVIQ_NONE;
    unsigned long flags;
    int ret, i, offset, avail;

    mutex_lock(&tx_ring->long_port.mutex);

    /* check slot available in the SW queue */
    while (axiomnet_long_tx_avail(tx_ring) == 0) { /* no space to write */
        drvdata->stats.wait_long_tx++;
        mutex_unlock(&tx_ring->long_port.mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(tx_ring->long_port.wait_queue,
                    axiomnet_long_tx_avail(tx_ring) != 0))
            return -ERESTARTSYS;

        mutex_lock(&tx_ring->long_port.mutex);
    }

    spin_lock_irqsave(&long_queue->queue_lock, flags);
    queue_slot = eviq_free_pop(&long_queue->evi_queue);
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);

    mutex_unlock(&tx_ring->long_port.mutex);

    /* impossible */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        ret = -EFAULT;
        goto err_nopush;
    }

    long_msg = &(long_queue->queue_desc[queue_slot]);

    /* copy relevant long field from user space */
    long_msg->header.tx.port_type = user_header->tx.port_type;
    long_msg->header.tx.dst = user_header->tx.dst;
    long_msg->header.tx.payload_size = user_header->tx.payload_size;

    if (unlikely(user_header->tx.payload_size > AXIOM_LONG_PAYLOAD_MAX_SIZE)) {
        ret = -EFBIG;
        goto err;
    }

    /* copy the payload in the TX buffer */
    offset = 0;
    for (i = 0; i < iovcnt; i++) {
        int copied = iov[i].iov_len;

        if ((copied + offset) > user_header->tx.payload_size) {
            ret = -EFBIG;
            EPRINTF("iov[%d] - iovcnt: %d psize: %d offset: %d copied: %d",
                    i, iovcnt, user_header->tx.payload_size, offset, copied);
            goto err;
        }

        ret = axiom_copy_from_user((uint8_t *)(long_msg->payload) + offset,
                iov[i].iov_base, copied);
        if (unlikely(ret)) {
            ret = -EFAULT;
            goto err;
        }

        offset += copied;
    }

    /* callback to free the buffer when the ack is received */
    cb.func = axiomnet_long_callback;
    cb.data = (void *)(uintptr_t)queue_slot;

    ret = axiomnet_rdma_tx(filep, &(long_msg->header), NULL, &cb, 0);
    if (ret < 0) {
        mutex_lock(&tx_ring->long_port.mutex);
        drvdata->stats.err_long_tx++;
        mutex_unlock(&tx_ring->long_port.mutex);

        EPRINTF("axiomnet_rdma_tx error");
        goto err;
    }

    mutex_lock(&tx_ring->long_port.mutex);

    drvdata->stats.pkt_long_tx++;
    drvdata->stats.bytes_long_tx += long_msg->header.tx.payload_size;

    mutex_unlock(&tx_ring->long_port.mutex);

err_nopush:
    return ret;

err:
    spin_lock_irqsave(&long_queue->queue_lock, flags);
    avail = eviq_free_avail(&long_queue->evi_queue);
    eviq_free_push(&long_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);
    /* send a notification to other thread */
    if (avail == 0)
        wake_up(&(tx_ring->long_port.wait_queue));

    return ret;
}

inline static ssize_t axiomnet_long_recv(struct file *filep,
        axiom_rdma_hdr_t *header, const struct iovec *iov, int iovcnt)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_rx_hwring *rx_ring = &drvdata->rdma_rx_ring;
    struct axiomnet_long_buf_lut *long_buf_lut = NULL;
    int port = priv->bind_port, ret;
    int i, offset, avail;
    ssize_t len;

    struct axiomnet_long_queue *long_queue = &rx_ring->long_queue;
    axiom_long_msg_t *long_msg;
    eviq_pnt_t queue_slot;
    unsigned long flags;

    /* check bind */
    if (unlikely(port == AXIOMNET_PORT_INVALID)) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    /* we have one mutex per port */
    mutex_lock(&rx_ring->long_ports[port].mutex);

    while (axiomnet_long_rx_avail(rx_ring, port) == 0) { /* nothing to read */
        drvdata->stats.wait_long_rx++;
        mutex_unlock(&rx_ring->long_ports[port].mutex);

        /* no blocking write */
        if (filep->f_flags & O_NONBLOCK)
            return -EAGAIN;

        /* put the process in the wait_queue to wait new space (irq) */
        if (wait_event_interruptible(rx_ring->long_ports[port].wait_queue,
                    axiomnet_long_rx_avail(rx_ring, port) != 0))
            return -ERESTARTSYS;

        mutex_lock(&rx_ring->long_ports[port].mutex);
    }

    /* copy packet from the ring */
    spin_lock_irqsave(&long_queue->queue_lock, flags);
    queue_slot = eviq_dequeue(&long_queue->evi_queue, port);
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->long_ports[port].mutex);

    /* XXX: impossible! */
    if (unlikely(queue_slot == EVIQ_NONE)) {
        len = -EFAULT;
        goto err;
    }
    DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);

    long_msg = &(long_queue->queue_desc[queue_slot]);

    /* find the long buffer where the payload is stored */
    long_buf_lut = axiomnet_long_rdma2buf(drvdata, long_msg->header.rx.dst_addr);
    if (unlikely(!long_buf_lut)) {
        EPRINTF("invalid dst_addr: 0x%x", long_msg->header.rx.dst_addr);
        len = -EFAULT;
        goto free_enqueue;
    }

    if (unlikely(header->rx.payload_size < long_msg->header.rx.payload_size)) {
        EPRINTF("payload received too big - payload: available %d - received %d",
                header->rx.payload_size, long_msg->header.rx.payload_size);
        len = -EFBIG;
        goto free_enqueue;
    }

    memcpy(header, &(long_msg->header), sizeof(*header));

    offset = 0;
    for (i = 0; (i < iovcnt) &&
            (offset < long_msg->header.rx.payload_size); i++) {
        int copied = min((int)(iov[i].iov_len),
                (int)(long_msg->header.rx.payload_size - offset));

        ret = axiom_copy_to_user(iov[i].iov_base,
                (uint8_t *)(long_buf_lut->long_buf_sw) + offset, copied);
        if (unlikely(ret)) {
            len = -EFAULT;
            goto free_enqueue;
        }

        offset += copied;
    }

    len = sizeof(*header) + long_msg->header.rx.payload_size;

free_enqueue:
    spin_lock_irqsave(&long_queue->queue_lock, flags);
    avail = eviq_free_avail(&long_queue->evi_queue);
    eviq_free_push(&long_queue->evi_queue, queue_slot);
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);
    /* send a notification to kthread */
    if (avail)
        axiom_kthread_wakeup(&drvdata->kthread_rdma);

    mutex_lock(&rx_ring->long_ports[port].mutex);
    if (long_buf_lut) {
        /* free the buffer for the HW, copying the initialization structure */
        axiom_hw_set_long_buf(drvdata->dev_api, long_buf_lut->buf_id,
                &long_buf_lut->long_buf_hw);
    }
    mutex_unlock(&rx_ring->long_ports[port].mutex);

err:

    DPRINTF("end len:%zu", len);
    return len;
}

static long axiomnet_long_flush(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_rx_hwring *rx_ring = &drvdata->rdma_rx_ring;
    struct axiomnet_long_queue *long_queue = &rx_ring->long_queue;
    int port = priv->bind_port;
    unsigned long flags;
    long ret = 0;

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    mutex_lock(&rx_ring->long_ports[port].mutex);

    /* take the lock to avoid enqueue during the flush */
    spin_lock_irqsave(&long_queue->queue_lock, flags);

    while (axiomnet_long_rx_avail(rx_ring, port) != 0) {
        eviq_pnt_t queue_slot;
        axiom_long_msg_t *long_msg;
        struct axiomnet_long_buf_lut *long_buf_lut;

        queue_slot = eviq_dequeue(&long_queue->evi_queue, port);
        /* XXX: impossible! */
        if (queue_slot == EVIQ_NONE) {
            ret = -EFAULT;
            goto err;
        }

        long_msg = &(long_queue->queue_desc[queue_slot]);

        /* find the long buffer where the payload is stored */
        long_buf_lut = axiomnet_long_rdma2buf(drvdata, long_msg->header.rx.dst_addr);
        if (unlikely(!long_buf_lut)) {
            EPRINTF("invalid dst_addr: 0x%x", long_msg->header.rx.dst_addr);
            ret = -EFAULT;
            goto err;
        }

        eviq_free_push(&long_queue->evi_queue, queue_slot);

        /* free the buffer for the HW, copying the initialization structure */
        axiom_hw_set_long_buf(drvdata->dev_api, long_buf_lut->buf_id,
                &long_buf_lut->long_buf_hw);


        DPRINTF("queue remove - queue_slot: %d port: %d", queue_slot, port);
    }

err:
    spin_unlock_irqrestore(&long_queue->queue_lock, flags);

    mutex_unlock(&rx_ring->long_ports[port].mutex);

    axiom_kthread_wakeup(&drvdata->kthread_rdma);

    return ret;
}

/**************************** Worker functions ********************************/
static void axiomnet_raw_rx_worker(void *data)
{
    struct axiomnet_raw_rx_hwring *rx_ring = data;

    /* fetch raw rx queue elements */
    axiom_raw_rx_dequeue(rx_ring);
}

static void axiomnet_rdma_rx_worker(void *data)
{
    struct axiomnet_rdma_rx_hwring *rx_ring = data;

    /* fetch rdma rx queue elements */
    axiom_rdma_rx_dequeue(rx_ring);
}

inline static bool axiomnet_watchdog_work_todo(void *data)
{
    if (watchdog_period == 0)
        return false;

    return true;
}

static void axiomnet_watchdog_worker(void *data)
{
    struct axiomnet_drvdata *drvdata = (struct axiomnet_drvdata *) data;

    if (axiomnet_raw_rx_work_todo(&drvdata->raw_rx_ring)) {
        axiom_kthread_wakeup(&(drvdata->kthread_raw));
    }

    if (axiomnet_raw_tx_avail(&drvdata->raw_tx_ring)) {
        wake_up(&(drvdata->raw_tx_ring.port.wait_queue));
    }

    if (axiomnet_rdma_rx_work_todo(&drvdata->rdma_rx_ring)) {
        axiom_kthread_wakeup(&(drvdata->kthread_rdma));
    }

    if (axiomnet_rdma_tx_avail(&drvdata->rdma_tx_ring)) {
        wake_up(&(drvdata->rdma_tx_ring.rdma_port.wait_queue));
    }

    msleep(watchdog_period);
}

/***************************** Init functions *********************************/

static void axiomnet_raw_rx_hwring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_raw_rx_hwring *rx_ring)
{
    if (rx_ring->sw_queue.queue_desc) {
        kfree(rx_ring->sw_queue.queue_desc);
        rx_ring->sw_queue.queue_desc = NULL;
    }

    eviq_release(&rx_ring->sw_queue.evi_queue);
}

static int axiomnet_raw_rx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_raw_rx_hwring *rx_ring)
{
    int err, port;

    rx_ring->drvdata = drvdata;

    for (port = 0; port < AXIOM_PORT_NUM; port++) {
        mutex_init(&rx_ring->ports[port].mutex);
        init_waitqueue_head(&rx_ring->ports[port].wait_queue);
    }

    spin_lock_init(&rx_ring->sw_queue.queue_lock);

    err = eviq_init(&rx_ring->sw_queue.evi_queue, AXIOMNET_RAW_QUEUE_NUM,
            AXIOMNET_RAW_QUEUE_FREE_LEN);
    if (err) {
        err = -ENOMEM;
        goto err;
    }

    rx_ring->sw_queue.queue_desc = kcalloc(AXIOMNET_RAW_QUEUE_FREE_LEN,
            sizeof(*(rx_ring->sw_queue.queue_desc)), GFP_KERNEL);
    if (rx_ring->sw_queue.queue_desc == NULL) {
        err = -ENOMEM;
        goto release_eviq;
    }

    return 0;

release_eviq:
    eviq_release(&rx_ring->sw_queue.evi_queue);
err:
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_raw_tx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_raw_tx_hwring *tx_ring)
{
    tx_ring->drvdata = drvdata;

    mutex_init(&tx_ring->port.mutex);
    init_waitqueue_head(&tx_ring->port.wait_queue);
    return 0;
}

static void axiomnet_long_queue_release(struct axiomnet_long_queue *long_queue)
{
    if (long_queue->queue_desc) {
        kfree(long_queue->queue_desc);
        long_queue->queue_desc = NULL;
    }

    eviq_release(&long_queue->evi_queue);
}

static int axiomnet_long_queue_init(struct axiomnet_long_queue *long_queue,
        int queues, int free_elem)
{
    int err;

    /* init LONG queue */
    spin_lock_init(&long_queue->queue_lock);

    err = eviq_init(&long_queue->evi_queue, queues, free_elem);
    if (err) {
        err = -ENOMEM;
        return err;
    }

    long_queue->queue_desc = kcalloc(free_elem,
            sizeof(*(long_queue->queue_desc)), GFP_KERNEL);
    if (long_queue->queue_desc == NULL) {
        err = -ENOMEM;
        goto release_long_eviq;
    }

    return 0;

release_long_eviq:
    eviq_release(&long_queue->evi_queue);

    return err;
}

static void axiomnet_rdma_rx_hwring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_rdma_rx_hwring *rx_ring)
{
    axiomnet_long_queue_release(&rx_ring->long_queue);
}

static void axiomnet_rdma_tx_hwring_release(struct axiomnet_drvdata *drvdata,
            struct axiomnet_rdma_tx_hwring *tx_ring)
{
    axiomnet_long_queue_release(&tx_ring->long_queue);

    if (tx_ring->rdma_queue.queue_desc) {
        kfree(tx_ring->rdma_queue.queue_desc);
        tx_ring->rdma_queue.queue_desc = NULL;
    }

    eviq_release(&tx_ring->rdma_queue.evi_queue);
}

static int axiomnet_rdma_rx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_rdma_rx_hwring *rx_ring)
{
    int err, port;

    rx_ring->drvdata = drvdata;
    rx_ring->tx_rdma_queue = &(drvdata->rdma_tx_ring.rdma_queue);

    /* init LONG queue */
    for (port = 0; port < AXIOM_PORT_NUM; port++) {
        mutex_init(&rx_ring->long_ports[port].mutex);
        init_waitqueue_head(&rx_ring->long_ports[port].wait_queue);
    }

    err = axiomnet_long_queue_init(&rx_ring->long_queue,
            AXIOMNET_LONG_RXQUEUE_NUM, AXIOMNET_LONG_RXQUEUE_FREE_LEN);
    if (err) {
        return err;
    }

    return 0;
}

static int axiomnet_rdma_tx_hwring_init(struct axiomnet_drvdata *drvdata,
        struct axiomnet_rdma_tx_hwring *tx_ring)
{
    int err, i;

    tx_ring->drvdata = drvdata;

    /* init RDMA queue */
    mutex_init(&tx_ring->rdma_port.mutex);
    init_waitqueue_head(&tx_ring->rdma_port.wait_queue);

    spin_lock_init(&tx_ring->rdma_queue.queue_lock);

    err = eviq_init(&tx_ring->rdma_queue.evi_queue, AXIOMNET_RDMA_QUEUE_NUM,
            AXIOMNET_RDMA_QUEUE_FREE_LEN);
    if (err) {
        err = -ENOMEM;
        goto err;
    }

    tx_ring->rdma_queue.queue_desc = kcalloc(AXIOMNET_RDMA_QUEUE_FREE_LEN,
            sizeof(*(tx_ring->rdma_queue.queue_desc)), GFP_KERNEL);
    if (tx_ring->rdma_queue.queue_desc == NULL) {
        err = -ENOMEM;
        goto release_rdma_eviq;
    }

    for (i = 0; i < AXIOMNET_RDMA_QUEUE_FREE_LEN; i++) {
        tx_ring->rdma_queue.queue_desc[i].msg_id = i;
        tx_ring->rdma_queue.queue_desc[i].msg_id_counter = 0;
        tx_ring->rdma_queue.queue_desc[i].header.tx.dst = AXIOM_NULL_NODE;
        init_waitqueue_head(&(tx_ring->rdma_queue.queue_desc[i].wait_queue));
    }

    /* init LONG queue */
    mutex_init(&tx_ring->long_port.mutex);
    init_waitqueue_head(&tx_ring->long_port.wait_queue);

    err = axiomnet_long_queue_init(&tx_ring->long_queue,
            AXIOMNET_LONG_TXQUEUE_NUM, AXIOMNET_LONG_TXQUEUE_FREE_LEN);
    if (err) {
        goto free_rdma_queue;
    }

    return 0;

free_rdma_queue:
    kfree(tx_ring->rdma_queue.queue_desc);
    tx_ring->rdma_queue.queue_desc = NULL;
release_rdma_eviq:
    eviq_release(&tx_ring->rdma_queue.evi_queue);
err:
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_rdma_release(struct axiomnet_drvdata *drvdata)
{
    iounmap(drvdata->long_vaddr);
    drvdata->long_vaddr = NULL;
    drvdata->dma_paddr = 0;
    drvdata->dma_size = 0;
    drvdata->rdma_paddr = 0;
    drvdata->rdma_size = 0;
    drvdata->long_paddr = 0;
    drvdata->long_size = 0;
}

static int axiomnet_rdma_init(struct axiomnet_drvdata *drvdata)
{
    unsigned long mem_app_base, mem_nic_base;
    size_t mem_app_size, mem_nic_size;
    int ret, i;

    /*  ___________________________
     * |                           | mem_app_base
     * |                           |
     * |                           |
     * |         RDMA zone         |
     * |                           |
     * |                           |
     * |___________________________|
     * |                           | mem_nic_base
     * |    LONG Rx Buf (32x64k)   |
     * |___________________________|
     * |                           |
     * |    LONG Tx Buf (32x64k)   |
     * |___________________________|
     *
     */
    if (axiom_mem_dev_get_appspace(&mem_app_base, &mem_app_size)) {
        ret = -EFAULT;
        goto err;
    }
    IPRINTF(1, "RDMA APP physical addr: 0x%lx size:%zu", mem_app_base,
            mem_app_size);

    if (axiom_mem_dev_get_nicspace(&mem_nic_base, &mem_nic_size)) {
        ret = -EFAULT;
        goto err;
    }
    IPRINTF(1, "RDMA NIC physical addr: 0x%lx size:%zu", mem_nic_base,
            mem_nic_size);

    drvdata->rdma_paddr = mem_app_base;
    drvdata->rdma_size = mem_app_size;
    drvdata->long_paddr = mem_nic_base;
    drvdata->long_size = 2 * (AXIOMREG_LEN_LONG_BUF * AXIOM_LONG_PAYLOAD_MAX_SIZE);
    drvdata->dma_paddr = drvdata->rdma_paddr;
    drvdata->dma_size = drvdata->rdma_size + drvdata->long_size;

    if (drvdata->long_size > mem_nic_size) {
        ret = -EFAULT;
        EPRINTF("No space available to allocate LONG buffers. \
                [avail: %lu required: %llu", mem_nic_size, drvdata->long_size);
        goto err;
    }

    drvdata->long_vaddr = ioremap(drvdata->long_paddr, drvdata->long_size);
    drvdata->long_rx_vaddr = drvdata->long_vaddr;
    drvdata->long_tx_vaddr = drvdata->long_rx_vaddr +
        (AXIOMREG_LEN_LONG_BUF * AXIOM_LONG_PAYLOAD_MAX_SIZE);

    IPRINTF(1, "DMA private NIC mapped - vaddr 0x%p paddr 0x%llx size 0x%llx",
            drvdata->long_vaddr, drvdata->long_paddr, drvdata->long_size);

    axiom_hw_set_rdma_zone(drvdata->dev_api, drvdata->dma_paddr,
            drvdata->dma_paddr + drvdata->dma_size - 1);

    /* LONG RX buffers */
    for (i = 0; i < AXIOMREG_LEN_LONG_BUF; i++) {
        struct axiomnet_long_buf_lut *long_buf_lut =
            &drvdata->long_rxbuf_lut[i];

        long_buf_lut->buf_id = i;
        long_buf_lut->long_buf_sw = drvdata->long_rx_vaddr +
            (i * AXIOM_LONG_PAYLOAD_MAX_SIZE);

        long_buf_lut->long_buf_hw.field.address = drvdata->rdma_size +
            (i * AXIOM_LONG_PAYLOAD_MAX_SIZE);
        long_buf_lut->long_buf_hw.field.size = AXIOM_LONG_PAYLOAD_MAX_SIZE;
        long_buf_lut->long_buf_hw.field.msg_id = 0xFF;
        long_buf_lut->long_buf_hw.field.flags = AXIOMREG_LONG_BUF_FREE;

        /* set buf in the HW */
        axiom_hw_set_long_buf(drvdata->dev_api, i, &long_buf_lut->long_buf_hw);
    }

    /* LONG TX buffers */
    for (i = 0; i < AXIOMREG_LEN_LONG_BUF; i++) {
        /* virtual address of RDMA where TX buffers are mapped */
        drvdata->rdma_tx_ring.long_queue.queue_desc[i].payload =
            drvdata->long_tx_vaddr + (i * AXIOM_LONG_PAYLOAD_MAX_SIZE);

        /* offset in the RDMA zone */
        drvdata->rdma_tx_ring.long_queue.queue_desc[i].header.tx.src_addr =
            drvdata->rdma_size +
            (AXIOMREG_LEN_LONG_BUF * AXIOM_LONG_PAYLOAD_MAX_SIZE) +
            (i * AXIOM_LONG_PAYLOAD_MAX_SIZE);
    }

    return 0;
err:
    DPRINTF("error: %d", ret);
    return ret;
}

int axiomnet_probe(struct axiomnet_drvdata *drvdata,
    axiom_dev_t *dev_api)
{
    int err = 0;

    DPRINTF("start");

    memset(drvdata, 0, sizeof(*drvdata));

    drvdata->dev_api = dev_api;
    mutex_init(&drvdata->lock);

    /* TODO: check version */
    err = axiom_hw_check_version(drvdata->dev_api);
    if (err) {
        EPRINTF("check version failed\n");
        return err;
    }

    if (verbose > 1) {
        axiom_print_status_reg(drvdata->dev_api);
        axiom_print_control_reg(drvdata->dev_api);
        axiom_print_routing_reg(drvdata->dev_api);
        axiom_print_queue_reg(drvdata->dev_api);
    }

    /* alloc char device */
    err = axiomnet_alloc_chrdev(drvdata, &chrdev);
    if (err) {
        EPRINTF("could not alloc char dev\n");
        return err;
    }

    /* init RAW TX ring */
    err = axiomnet_raw_tx_hwring_init(drvdata, &drvdata->raw_tx_ring);
    if (err) {
        EPRINTF("could not init raw TX ring\n");
        goto free_cdev;
    }

    /* init RDMA TX ring */
    err = axiomnet_rdma_tx_hwring_init(drvdata, &drvdata->rdma_tx_ring);
    if (err) {
        EPRINTF("could not init RDMA TX ring\n");
        goto free_cdev;
    }

    /* init RAW RX ring */
    err = axiomnet_raw_rx_hwring_init(drvdata, &drvdata->raw_rx_ring);
    if (err) {
        EPRINTF("could not init raw RX ring\n");
        goto free_tx_ring;
    }

    /* init RDMA RX ring */
    err = axiomnet_rdma_rx_hwring_init(drvdata, &drvdata->rdma_rx_ring);
    if (err) {
        EPRINTF("could not init RDMA RX ring\n");
        goto free_rx_ring;
    }

    /* init RAW kthread */
    err = axiom_kthread_init(&drvdata->kthread_raw, axiomnet_raw_rx_worker,
            axiomnet_raw_rx_work_todo, &drvdata->raw_rx_ring, "RAW kthread");
    if (err) {
        EPRINTF("could not init kthread\n");
        goto free_rdma_rx_ring;
    }

    /* init RDMA kthread */
    err = axiom_kthread_init(&drvdata->kthread_rdma, axiomnet_rdma_rx_worker,
            axiomnet_rdma_rx_work_todo, &drvdata->rdma_rx_ring, "RDMA kthread");
    if (err) {
        EPRINTF("could not init kthread\n");
        goto free_raw_kthread;
    }

    /* init RDMA */
    err = axiomnet_rdma_init(drvdata);
    if (err) {
        EPRINTF("could not init RDMA zone\n");
        goto free_rdma_kthread;
    }

    axiom_hw_enable_irq(drvdata->dev_api);

    /* init watchdog kthread */
    err = axiom_kthread_init(&drvdata->kthread_wtd, axiomnet_watchdog_worker,
            axiomnet_watchdog_work_todo, drvdata, "WTD kthread");
    if (err) {
        EPRINTF("could not init kthread\n");
        goto free_rdma;
    }

    DPRINTF("end");

    return 0;
free_rdma:
    axiom_hw_disable_irq(drvdata->dev_api);
    axiomnet_rdma_release(drvdata);
free_rdma_kthread:
    axiom_kthread_uninit(&drvdata->kthread_rdma);
free_raw_kthread:
    axiom_kthread_uninit(&drvdata->kthread_raw);
free_rdma_rx_ring:
    axiomnet_rdma_rx_hwring_release(drvdata, &drvdata->rdma_rx_ring);
free_rx_ring:
    axiomnet_raw_rx_hwring_release(drvdata, &drvdata->raw_rx_ring);
free_tx_ring:
    axiomnet_rdma_tx_hwring_release(drvdata, &drvdata->rdma_tx_ring);
free_cdev:
    axiomnet_destroy_chrdev(drvdata, &chrdev);

    DPRINTF("error: %d", err);
    return err;
}

int axiomnet_remove(struct axiomnet_drvdata *drvdata)
{
    DPRINTF("start");

    axiom_hw_disable_irq(drvdata->dev_api);

    axiomnet_rdma_release(drvdata);

    axiom_kthread_uninit(&drvdata->kthread_wtd);
    axiom_kthread_uninit(&drvdata->kthread_rdma);
    axiom_kthread_uninit(&drvdata->kthread_raw);

    axiomnet_rdma_rx_hwring_release(drvdata, &drvdata->rdma_rx_ring);
    axiomnet_raw_rx_hwring_release(drvdata, &drvdata->raw_rx_ring);
    axiomnet_rdma_tx_hwring_release(drvdata, &drvdata->rdma_tx_ring);

    axiomnet_destroy_chrdev(drvdata, &chrdev);
    DPRINTF("end");
    return 0;
}

/****************************** Ports Handling  *******************************/

static void axiomnet_unbind(struct axiomnet_priv *priv) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;

    /* check if the process bound some port */
    if (priv->bind_port == AXIOMNET_PORT_INVALID ||
            priv->bind_port > AXIOM_PORT_MAX) {
        return;
    }

    mutex_lock(&drvdata->lock);
    if (priv->type == AXNET_FDTYPE_RAW)
        drvdata->raw_rx_ring.port_used &= ~(1 << (uint8_t)(priv->bind_port));
    if (priv->type == AXNET_FDTYPE_LONG)
        drvdata->rdma_rx_ring.port_used &= ~(1 << (uint8_t)(priv->bind_port));
    mutex_unlock(&drvdata->lock);

    priv->bind_port = AXIOMNET_PORT_INVALID;
}

static long axiomnet_bind(struct axiomnet_priv *priv, uint8_t *port) {
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    long ret = 0;
    uint8_t port_used, port_set;

    /* unbind previous bind */
    axiomnet_unbind(priv);

    mutex_lock(&drvdata->lock);

    if (priv->type == AXNET_FDTYPE_RAW) {
        port_used = drvdata->raw_rx_ring.port_used;
    } else if (priv->type == AXNET_FDTYPE_LONG) {
        port_used = drvdata->rdma_rx_ring.port_used;
    } else {
        EPRINTF("bind not allowed on this file descriptor");
        ret = -EFAULT;
        goto exit;
    }

    if (*port == AXIOM_PORT_ANY) {
        int i;
        /* assign first port available */
        for (i = 0; i < AXIOM_PORT_NUM; i++) {
            if (!((1 << i) & port_used)) {
                *port = i;
                break;
            }
        }

        if (*port == AXIOM_PORT_ANY) {
            ret = -EBUSY;
            goto exit;
        }
    } else if (*port >= AXIOM_PORT_NUM) {
        ret = -EFBIG;
        goto exit;
    }


    DPRINTF("port: 0x%x port_used: 0x%x", *port, port_used);

    /* check if port is already bound */
    if (((1 << *port) & port_used)) {
        EPRINTF("Port %d already bound", *port);
        ret = -EBUSY;
        goto exit;
    }

    priv->bind_port = *port;
    port_set = (1 << (uint8_t)*port);

    if (priv->type == AXNET_FDTYPE_RAW) {
        drvdata->raw_rx_ring.port_used |= port_set;
    } else if (priv->type == AXNET_FDTYPE_LONG) {
        drvdata->rdma_rx_ring.port_used |= port_set;
    }

    DPRINTF("port: 0x%x", *port);

exit:
    mutex_unlock(&drvdata->lock);

    return ret;
}

inline static int axiomnet_check_port(struct axiomnet_priv *priv)
{
    int port = priv->bind_port;

    /* check bind */
    if (port == AXIOMNET_PORT_INVALID) {
        EPRINTF("port not assigned");
        return -EFAULT;
    }

    return port;
}

/************************* AxiomNet Debug info  *******************************/

void axiomnet_debug_raw(struct axiomnet_drvdata *drvdata)
{
    struct axiomnet_raw_tx_hwring *tx_ring = &drvdata->raw_tx_ring;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    int i;

    printk(KERN_ERR "---- AXIOM DEBUG RAW ----\n");
    printk(KERN_ERR "  tx-avail [HW]: %u\n",
            axiom_hw_raw_tx_avail(drvdata->dev_api));
    printk(KERN_ERR "  tx-avail [SW]: %u\n\n",
            axiomnet_raw_tx_avail(tx_ring));

    printk(KERN_ERR "  rx-avail [HW]: %u\n",
            axiom_hw_raw_rx_avail(drvdata->dev_api));
    printk(KERN_ERR "  rx-avail [SW] free_slot: %u\n",
            eviq_free_avail(&rx_ring->sw_queue.evi_queue));
    for (i = 0; i < AXIOM_PORT_NUM; i++) {
        printk(KERN_ERR "  rx-avail[%d] [SW]: %d\n", i,
                axiomnet_raw_rx_avail(rx_ring, i));
    }

}

void axiomnet_debug_long(struct axiomnet_drvdata *drvdata)
{
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_rx_hwring *rx_ring = &drvdata->rdma_rx_ring;
    struct axiomnet_long_queue *long_queue = &rx_ring->long_queue;
    int i;

    printk(KERN_ERR "---- AXIOM DEBUG LONG ----\n");
    printk(KERN_ERR "  tx-avail [SW]: %d\n\n",
            axiomnet_long_tx_avail(tx_ring));

    printk(KERN_ERR "  rx-avail [SW] free_slot: %u\n",
            eviq_free_avail(&long_queue->evi_queue));
    for (i = 0; i < AXIOM_PORT_NUM; i++) {
        printk(KERN_ERR "  rx-avail[%d] [SW]: %d\n", i,
                axiomnet_long_rx_avail(rx_ring, i));
    }

}

void axiomnet_debug_rdma(struct axiomnet_drvdata *drvdata)
{
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_rx_hwring *rx_ring = &drvdata->rdma_rx_ring;
    int i;

    printk(KERN_ERR "---- AXIOM DEBUG RDMA ----\n");
    printk(KERN_ERR "  tx-avail [HW]: %u\n",
            axiom_hw_rdma_tx_avail(drvdata->dev_api));
    printk(KERN_ERR "  tx-avail [SW]: %d\n\n",
            axiomnet_rdma_tx_avail(tx_ring));

    printk(KERN_ERR "  rx-avail [HW]: %u\n\n",
            axiom_hw_rdma_rx_avail(drvdata->dev_api));

    for (i = 0; i < AXIOMNET_RDMA_QUEUE_FREE_LEN; i++) {
        axiom_rdma_status_t *rdma_status =
            &(rx_ring->tx_rdma_queue->queue_desc[i]);
        printk(KERN_ERR "  rdma_status[%d] - ack_wait: 0x%x ack_recv: 0x%x "
                "rid: 0x%x\n", i, rdma_status->ack_waiting,
                rdma_status->ack_received, rdma_status->header.tx.dst);
    }
}


int axiomnet_debug_info(struct axiomnet_drvdata *drvdata, uint32_t flags)
{
    if (!drvdata)
        return -EINVAL;

    printk(KERN_ERR "\n---- AXIOM NIC DEBUG ----\n\n");

    if (flags & AXIOM_DEBUG_FLAG_STATUS)
        axiom_print_status_reg(drvdata->dev_api);
    if (flags & AXIOM_DEBUG_FLAG_CONTROL)
        axiom_print_control_reg(drvdata->dev_api);
    if (flags & AXIOM_DEBUG_FLAG_ROUTING)
        axiom_print_routing_reg(drvdata->dev_api);
    if (flags & AXIOM_DEBUG_FLAG_QUEUES)
        axiom_print_queue_reg(drvdata->dev_api);
    if (flags & AXIOM_DEBUG_FLAG_FPGA)
        axiom_print_fpga_debug(drvdata->dev_api);

    printk(KERN_ERR "\n---- AXIOM DRIVER DEBUG ----\n\n");
    if (flags & AXIOM_DEBUG_FLAG_RAW)
        axiomnet_debug_raw(drvdata);
    if (flags & AXIOM_DEBUG_FLAG_LONG)
        axiomnet_debug_long(drvdata);
    if (flags & AXIOM_DEBUG_FLAG_RDMA)
        axiomnet_debug_rdma(drvdata);

    return 0;
}

/************************ AxiomNet Char Device  ******************************/

static unsigned int axiomnet_poll_raw(struct file *filep, poll_table *wait)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_raw_tx_hwring *tx_ring = &drvdata->raw_tx_ring;
    struct axiomnet_raw_rx_hwring *rx_ring = &drvdata->raw_rx_ring;
    int port = priv->bind_port;
    unsigned int ret = 0;

    poll_wait(filep, &tx_ring->port.wait_queue, wait);

    if (poll_requested_events(wait) & POLLOUT) {
        drvdata->stats.poll_raw_tx++;
        if (axiomnet_raw_tx_avail(tx_ring) != 0) { /* space to write */
            ret |= POLLOUT | POLLWRNORM;
            drvdata->stats.poll_avail_raw_tx++;
        }
    }

    if ((poll_requested_events(wait) & POLLIN) &&
            (port != AXIOMNET_PORT_INVALID)) {
        poll_wait(filep, &rx_ring->ports[port].wait_queue, wait);

        drvdata->stats.poll_raw_rx++;
        if (axiomnet_raw_rx_avail(rx_ring, port) != 0) { /* something to read */
            ret |= POLLIN | POLLRDNORM;
            drvdata->stats.poll_avail_raw_rx++;
        }
    }

    return ret;
}

static unsigned int axiomnet_poll_rdma(struct file *filep, poll_table *wait)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    unsigned int ret = 0;

    poll_wait(filep, &tx_ring->rdma_port.wait_queue, wait);

    if (poll_requested_events(wait) & POLLOUT) {
        drvdata->stats.poll_rdma_tx++;
        if (axiomnet_rdma_tx_avail(tx_ring) != 0) { /* space to write */
            ret |= POLLOUT | POLLWRNORM;
            drvdata->stats.poll_avail_rdma_tx++;
        }
    }

    return ret;
}

static unsigned int axiomnet_poll_long(struct file *filep, poll_table *wait)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    struct axiomnet_rdma_tx_hwring *tx_ring = &drvdata->rdma_tx_ring;
    struct axiomnet_rdma_rx_hwring *rx_ring = &drvdata->rdma_rx_ring;
    int port = priv->bind_port;
    unsigned int ret = 0;

    poll_wait(filep, &tx_ring->long_port.wait_queue, wait);
    poll_wait(filep, &tx_ring->rdma_port.wait_queue, wait);

    if (poll_requested_events(wait) & POLLOUT) {
        drvdata->stats.poll_long_tx++;
        if ((axiomnet_long_tx_avail(tx_ring) != 0) &&
                (axiomnet_rdma_tx_avail(tx_ring) != 0)) { /* space to write */
            ret |= POLLOUT | POLLWRNORM;
            drvdata->stats.poll_avail_long_tx++;
        }
    }

    if ((poll_requested_events(wait) & POLLIN) &&
            (port != AXIOMNET_PORT_INVALID)) {
        if (port != AXIOMNET_PORT_INVALID) {
            poll_wait(filep, &rx_ring->long_ports[port].wait_queue, wait);

            drvdata->stats.poll_long_rx++;
            if (axiomnet_long_rx_avail(rx_ring, port) != 0) { /* something to read */
                ret |= POLLIN | POLLRDNORM;
                drvdata->stats.poll_avail_long_rx++;
            }
        }
    }

    return ret;
}

static long axiomnet_ioctl_raw(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    void __user* argp = (void __user*)arg;
    axiom_ioctl_raw_t buf_raw;
    axiom_ioctl_raw_iov_t buf_raw_iov;
    axiom_ioctl_bind_t buf_bind;
    struct iovec iov[AXIOMNET_MAX_IOVEC];
    int buf_int, port;
    long ret = 0;

    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    switch (cmd) {
    case AXNET_BIND:
        ret = axiom_copy_from_user(&buf_bind, argp, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        ret = axiomnet_bind(priv, &(buf_bind.port));
        DPRINTF("bind port: %x flush: %x", priv->bind_port, buf_bind.flush);
        if (ret)
            return ret;
        /* flush all previous received packets */
        if (buf_bind.flush) {
            ret = axiomnet_raw_flush(priv);
            if (ret)
                return ret;
        }
        ret = axiom_copy_to_user(argp, &buf_bind, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_RAW:
        ret = axiom_copy_from_user(&buf_raw, argp, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        iov[0].iov_base = buf_raw.payload;
        iov[0].iov_len = buf_raw.header.tx.payload_size;
        ret = axiomnet_raw_send(filep, &(buf_raw.header), iov, 1);
        break;
    case AXNET_SEND_RAW_IOV:
        ret = axiom_copy_from_user(&buf_raw_iov, argp, sizeof(buf_raw_iov));
        if (ret)
            return -EFAULT;
        if (buf_raw_iov.iovcnt > AXIOMNET_MAX_IOVEC)
            return -EFBIG;
        ret = axiom_copy_from_user(&iov, buf_raw_iov.iov, buf_raw_iov.iovcnt *
                sizeof(buf_raw_iov.iov[0]));
        if (ret)
            return -EFAULT;
        ret = axiomnet_raw_send(filep, &(buf_raw_iov.header), iov,
                buf_raw_iov.iovcnt);
        break;
    case AXNET_RECV_RAW:
        ret = axiom_copy_from_user(&buf_raw, argp, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        iov[0].iov_base = buf_raw.payload;
        iov[0].iov_len = buf_raw.header.rx.payload_size;
        ret = axiomnet_raw_recv(filep, &(buf_raw.header), iov, 1);
        if (ret < 0)
            return ret;
        ret = axiom_copy_to_user(argp, &buf_raw, sizeof(buf_raw));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_RECV_RAW_IOV:
        ret = axiom_copy_from_user(&buf_raw_iov, argp, sizeof(buf_raw_iov));
        if (ret)
            return -EFAULT;
        if (buf_raw_iov.iovcnt > AXIOMNET_MAX_IOVEC)
            return -EFBIG;
        ret = axiom_copy_from_user(&iov, buf_raw_iov.iov, buf_raw_iov.iovcnt *
                sizeof(buf_raw_iov.iov[0]));
        if (ret)
            return -EFAULT;
        ret = axiomnet_raw_recv(filep, &(buf_raw_iov.header), iov,
                buf_raw_iov.iovcnt);
        if (ret < 0)
            return ret;
        ret = axiom_copy_to_user(argp, &buf_raw_iov, sizeof(buf_raw_iov));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_RAW_AVAIL:
        buf_int = axiomnet_raw_tx_avail(&drvdata->raw_tx_ring);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_RECV_RAW_AVAIL:
        port = axiomnet_check_port(priv);
        if (port < 0)
            return port;
        buf_int = axiomnet_raw_rx_avail(&drvdata->raw_rx_ring, port);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_FLUSH_RAW:
        ret = axiomnet_raw_flush(priv);
        break;
    default:
        ret = -EINVAL;
    }

    DPRINTF("end");
    return ret;
}

static long axiomnet_ioctl_long(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    void __user* argp = (void __user*)arg;
    axiom_ioctl_bind_t buf_bind;
    axiom_long_msg_t buf_long;
    axiom_ioctl_long_iov_t buf_long_iov;
    struct iovec iov[AXIOMNET_MAX_IOVEC];
    int buf_int, port;
    long ret = 0;

    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    switch (cmd) {
    case AXNET_BIND:
        ret = axiom_copy_from_user(&buf_bind, argp, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        ret = axiomnet_bind(priv, &(buf_bind.port));
        DPRINTF("bind port: %x flush: %x", priv->bind_port, buf_bind.flush);
        if (ret)
            return ret;
        /* flush all previous received packets */
        if (buf_bind.flush) {
            ret = axiomnet_long_flush(priv);
            if (ret)
                return ret;
        }
        ret = axiom_copy_to_user(argp, &buf_bind, sizeof(buf_bind));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_LONG:
        ret = axiom_copy_from_user(&buf_long, argp, sizeof(buf_long));
        if (ret)
            return -EFAULT;
        iov[0].iov_base = buf_long.payload;
        iov[0].iov_len = buf_long.header.tx.payload_size;
        ret = axiomnet_long_send(filep, &(buf_long.header), iov, 1);
        break;
    case AXNET_SEND_LONG_IOV:
        ret = axiom_copy_from_user(&buf_long_iov, argp, sizeof(buf_long_iov));
        if (ret)
            return -EFAULT;
        if (buf_long_iov.iovcnt > AXIOMNET_MAX_IOVEC)
            return -EFBIG;
        ret = axiom_copy_from_user(&iov, buf_long_iov.iov, buf_long_iov.iovcnt *
                sizeof(buf_long_iov.iov[0]));
        if (ret)
            return -EFAULT;
        ret = axiomnet_long_send(filep, &(buf_long_iov.header), iov,
                buf_long_iov.iovcnt);
        break;
    case AXNET_RECV_LONG:
        ret = axiom_copy_from_user(&buf_long, argp, sizeof(buf_long));
        if (ret)
            return -EFAULT;
        iov[0].iov_base = buf_long.payload;
        iov[0].iov_len = buf_long.header.rx.payload_size;
        ret = axiomnet_long_recv(filep, &(buf_long.header), iov, 1);
        if (ret < 0)
            return ret;
        ret = axiom_copy_to_user(argp, &buf_long, sizeof(buf_long));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_RECV_LONG_IOV:
        ret = axiom_copy_from_user(&buf_long_iov, argp, sizeof(buf_long_iov));
        if (ret)
            return -EFAULT;
        if (buf_long_iov.iovcnt > AXIOMNET_MAX_IOVEC)
            return -EFBIG;
        ret = axiom_copy_from_user(&iov, buf_long_iov.iov, buf_long_iov.iovcnt *
                sizeof(buf_long_iov.iov[0]));
        if (ret)
            return -EFAULT;
        ret = axiomnet_long_recv(filep, &(buf_long_iov.header), iov,
                buf_long_iov.iovcnt);
        if (ret < 0)
            return ret;
        ret = axiom_copy_to_user(argp, &buf_long_iov, sizeof(buf_long_iov));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_SEND_LONG_AVAIL:
        buf_int = axiomnet_long_tx_avail(&drvdata->rdma_tx_ring) &&
            axiomnet_rdma_tx_avail(&drvdata->rdma_tx_ring);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_RECV_LONG_AVAIL:
        port = axiomnet_check_port(priv);
        if (port < 0)
            return port;
        buf_int = axiomnet_long_rx_avail(&drvdata->rdma_rx_ring, port);
        put_user(buf_int, (int __user*)arg);
        break;
    case AXNET_FLUSH_LONG:
        ret = axiomnet_long_flush(priv);
        break;
    default:
        ret = -EINVAL;
    }

    DPRINTF("end");
    return ret;
}

static long axiomnet_ioctl_rdma(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    void __user* argp = (void __user*)arg;
    axiom_ioctl_rdma_t buf_rdma;
    axiom_ioctl_token_t buf_token;
    uint64_t buf_uint64;
    unsigned long buf_ulong;
    long ret = 0, err;

    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    switch (cmd) {
    case AXNET_RDMA_SIZE:
        buf_uint64 = drvdata->rdma_size;
        put_user(buf_uint64, (uint64_t __user*)arg);
        break;
    case AXNET_RDMA_WRITE:
    case AXNET_RDMA_READ:
        ret = axiom_copy_from_user(&buf_rdma, argp, sizeof(buf_rdma));
        if (ret)
            return -EFAULT;

        if (likely(priv->rdma_debug == 0)) {
            ret = axiom_mem_dev_virt2off(buf_rdma.app_id,
                    (unsigned long)(buf_rdma.src_addr),
                    buf_rdma.header.tx.payload_size,
                    &buf_ulong);
            if (ret) {
                EPRINTF("axiom_mem_dev_virt2off - ret %ld", ret);
                return -EFAULT;
            }
            buf_rdma.header.tx.src_addr = buf_ulong;

            ret = axiom_mem_dev_virt2off(buf_rdma.app_id,
                    (unsigned long)(buf_rdma.dst_addr),
                    buf_rdma.header.tx.payload_size,
                    &buf_ulong);
            if (ret) {
                EPRINTF("axiom_mem_dev_virt2off - ret %ld", ret);
                return -EFAULT;
            }
            buf_rdma.header.tx.dst_addr = buf_ulong;
        } else { /* if RDMA debug is enabled, we can't use the allocator API */
            buf_rdma.header.tx.src_addr = (unsigned long)(buf_rdma.src_addr);
            buf_rdma.header.tx.dst_addr = (unsigned long)(buf_rdma.dst_addr);
        }

        IPRINTF(verbose, "RDMA - src_offset: %d dst_offset: %d",
                buf_rdma.header.tx.src_addr, buf_rdma.header.tx.dst_addr);

        ret = axiomnet_rdma_tx(filep, &(buf_rdma.header), &(buf_rdma.token),
                NULL, buf_rdma.flags);
        if (ret < 0)
            return ret;

        err = axiom_copy_to_user(argp, &buf_rdma, sizeof(buf_rdma));
        if (err)
            return -EFAULT;
        break;
    case AXNET_RDMA_CHECK:
        ret = axiom_copy_from_user(&buf_token, argp, sizeof(buf_token));
        if (ret)
            return -EFAULT;

        ret = axiomnet_rdma_check(filep, &(buf_token));

        break;
    case AXNET_RDMA_WAIT:
        ret = axiom_copy_from_user(&buf_token, argp, sizeof(buf_token));
        if (ret)
            return -EFAULT;

        ret = axiomnet_rdma_wait(filep, &(buf_token));

        break;
    default:
        ret = -EINVAL;
    }

    DPRINTF("end");
    return ret;
}

static long axiomnet_ioctl_generic(struct file *filep, unsigned int cmd,
        unsigned long arg)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    void __user* argp = (void __user*)arg;
    uint32_t buf_uint32;
    uint8_t buf_uint8;
    uint8_t buf_uint8_2;
    axiom_ioctl_routing_t buf_routing;
    axiom_ioctl_debug_t buf_debug;
    long ret = 0;

    DPRINTF("start");

    if (!drvdata)
        return -EINVAL;

    switch (cmd) {
    case AXNET_SET_NODEID:
        get_user(buf_uint8, (uint8_t __user*)arg);
        axiom_hw_set_node_id(drvdata->dev_api, buf_uint8);
        drvdata->node_id = buf_uint8;
        break;
    case AXNET_GET_NODEID:
        buf_uint8 = axiom_hw_get_node_id(drvdata->dev_api);
        put_user(buf_uint8, (uint8_t __user*)arg);
        drvdata->node_id = buf_uint8;
        break;
    case AXNET_SET_ROUTING:
        ret = axiom_copy_from_user(&buf_routing, argp, sizeof(buf_routing));
        if (ret)
            return -EFAULT;
        ret = axiom_hw_set_routing(drvdata->dev_api, buf_routing.node_id,
                buf_routing.enabled_mask);
        if (ret)
            return -EFAULT;
        drvdata->routing_table[buf_routing.node_id] = buf_routing.enabled_mask;
        break;
    case AXNET_GET_ROUTING:
        ret = axiom_copy_from_user(&buf_routing, argp, sizeof(buf_routing));
        if (ret)
            return -EFAULT;
        ret = axiom_hw_get_routing(drvdata->dev_api, buf_routing.node_id,
                &buf_routing.enabled_mask);
        if (ret)
            return -EFAULT;
        drvdata->routing_table[buf_routing.node_id] = buf_routing.enabled_mask;
        ret = axiom_copy_to_user(argp, &buf_routing, sizeof(buf_routing));
        break;
    case AXNET_GET_IFNUMBER:
        ret = axiom_hw_get_if_number(drvdata->dev_api, &buf_uint8);
        put_user(buf_uint8, (uint8_t __user*)arg);
        break;
    case AXNET_GET_IFINFO:
        get_user(buf_uint8, (uint8_t __user*)arg);
        ret = axiom_hw_get_if_info(drvdata->dev_api, buf_uint8, &buf_uint8_2);
        put_user(buf_uint8_2, (uint8_t __user*)arg);
        break;
    case AXNET_GET_STATUS:
        buf_uint32 = axiom_hw_read_ni_status(drvdata->dev_api);
        put_user(buf_uint32, (uint32_t __user*)arg);
        break;
    case AXNET_GET_CONTROL:
        buf_uint32 = axiom_hw_read_ni_control(drvdata->dev_api);
        put_user(buf_uint32, (uint32_t __user*)arg);
        break;
    case AXNET_SET_CONTROL:
        get_user(buf_uint32, (uint32_t __user*)arg);
        axiom_hw_set_ni_control(drvdata->dev_api, buf_uint32);
        break;
    case AXNET_GET_STATS:
        ret = axiom_copy_to_user(argp, &drvdata->stats, sizeof(drvdata->stats));
        if (ret)
            return -EFAULT;
        break;
    case AXNET_DEBUG_INFO:
        ret = axiom_copy_from_user(&buf_debug, argp, sizeof(buf_debug));
        if (ret)
            return -EFAULT;
        ret = axiomnet_debug_info(drvdata, buf_debug.flags);
        break;
    default:
        ret = -EINVAL;
    }

    DPRINTF("end");
    return ret;
}

/* TO BE REMOVED: we use it only for debug to map all RDMA region */
static int axiomnet_mmap(struct file *filep, struct vm_area_struct *vma)
{
    struct axiomnet_priv *priv = filep->private_data;
    struct axiomnet_drvdata *drvdata = priv->drvdata;
    unsigned long size = vma->vm_end - vma->vm_start;
    int err = 0;
    DPRINTF("start");

    if (!drvdata || !drvdata->rdma_paddr)
        return -EINVAL;

    mutex_lock(&drvdata->lock);

    if (size != drvdata->rdma_size) {
        err= -EINVAL;
        goto err;
    }

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
    err = remap_pfn_range(vma, vma->vm_start,
            (drvdata->rdma_paddr >> PAGE_SHIFT) + vma->vm_pgoff, size,
            vma->vm_page_prot);
    if (err) {
        goto err;
    }

    /* RDMA debug enabled */
    priv->rdma_debug = 1;

    mutex_unlock(&drvdata->lock);

    DPRINTF("end");
    return 0;
err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to mmap registers [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_open_generic(struct inode *inode, struct file *filep)
{
    struct axiomnet_drvdata *drvdata = chrdev.drvdata;
    struct axiomnet_priv *priv;
    int err = 0;

    DPRINTF("start minor: %u drvdata: %p", iminor(inode), drvdata);

    /* allocate per-open structure and fill it out */
    priv = kzalloc(sizeof(*priv), GFP_KERNEL);
    if (priv == NULL)
        return -ENOMEM;

    /* set invalid port */
    priv->bind_port = AXIOMNET_PORT_INVALID;

    mutex_lock(&drvdata->lock);
    if (drvdata->used >= AXIOMNET_MAX_OPEN) {
        err = -EBUSY;
        goto err;
    }
    drvdata->used++;
    mutex_unlock(&drvdata->lock);

    priv->drvdata = drvdata;
    priv->type = AXNET_FDTYPE_GENERIC;

    filep->private_data = priv;

    DPRINTF("end");
    return 0;

err:
    mutex_unlock(&drvdata->lock);
    pr_err("unable to open char dev [error %d]\n", err);
    kfree(priv);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_open_raw(struct inode *inode, struct file *filep)
{
    struct axiomnet_priv *priv;
    int err;

    err = axiomnet_open_generic(inode, filep);
    if (err)
        return err;

    priv = filep->private_data;
    priv->type = AXNET_FDTYPE_RAW;

    return 0;
}

static int axiomnet_open_long(struct inode *inode, struct file *filep)
{
    struct axiomnet_priv *priv;
    int err;

    err = axiomnet_open_generic(inode, filep);
    if (err)
        return err;

    priv = filep->private_data;
    priv->type = AXNET_FDTYPE_LONG;

    return 0;
}

static int axiomnet_open_rdma(struct inode *inode, struct file *filep)
{
    struct axiomnet_priv *priv;
    int err;

    err = axiomnet_open_generic(inode, filep);
    if (err)
        return err;

    priv = filep->private_data;
    priv->type = AXNET_FDTYPE_RDMA;
    priv->rdma_debug = 0;

    return 0;
}

static int axiomnet_release(struct inode *inode, struct file *filep)
{
    struct axiomnet_drvdata *drvdata = chrdev.drvdata;
    struct axiomnet_priv *priv = filep->private_data;

    DPRINTF("start");

    axiomnet_unbind(priv);

    mutex_lock(&drvdata->lock);

    drvdata->used--;

    mutex_unlock(&drvdata->lock);

    filep->private_data = NULL;
    kfree(priv);

    DPRINTF("end");
    return 0;
}

static struct file_operations axiomnet_fops =
{
    .owner = THIS_MODULE,
    .open = axiomnet_open_generic,
    .release = axiomnet_release,
    .unlocked_ioctl = axiomnet_ioctl_generic,
};

static struct file_operations axiomnet_raw_fops =
{
    .owner = THIS_MODULE,
    .open = axiomnet_open_raw,
    .release = axiomnet_release,
    .unlocked_ioctl = axiomnet_ioctl_raw,
    .poll = axiomnet_poll_raw,
};

static struct file_operations axiomnet_long_fops =
{
    .owner = THIS_MODULE,
    .open = axiomnet_open_long,
    .release = axiomnet_release,
    .unlocked_ioctl = axiomnet_ioctl_long,
    .poll = axiomnet_poll_long,
};

static struct file_operations axiomnet_rdma_fops =
{
    .owner = THIS_MODULE,
    .open = axiomnet_open_rdma,
    .release = axiomnet_release,
    .unlocked_ioctl = axiomnet_ioctl_rdma,
    .poll = axiomnet_poll_rdma,
    .mmap = axiomnet_mmap,
};

static int axiomnet_alloc_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev)
{
    int err = 0;
    struct device *dev_ret;
    DPRINTF("start");

    /* create axiom device */
    dev_ret  = device_create(chrdev->dclass, NULL,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_MINOR), drvdata,
            "%s",AXIOMNET_DEV_NAME);
    if (IS_ERR(dev_ret)) {
        err = PTR_ERR(dev_ret);
        goto err;
    }

    /* create axiom device for raw messages */
    dev_ret  = device_create(chrdev->dclass, NULL,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_RAW_MINOR), drvdata,
            "%s",AXIOMNET_DEV_RAW_NAME);
    if (IS_ERR(dev_ret)) {
        err = PTR_ERR(dev_ret);
        goto free_dev;
    }

    /* create axiom device for long messages */
    dev_ret  = device_create(chrdev->dclass, NULL,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_LONG_MINOR), drvdata,
            "%s",AXIOMNET_DEV_LONG_NAME);
    if (IS_ERR(dev_ret)) {
        err = PTR_ERR(dev_ret);
        goto free_raw_dev;
    }

    /* create axiom device for rdma */
    dev_ret  = device_create(chrdev->dclass, NULL,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_RDMA_MINOR), drvdata,
            "%s",AXIOMNET_DEV_RDMA_NAME);
    if (IS_ERR(dev_ret)) {
        err = PTR_ERR(dev_ret);
        goto free_long_dev;
    }

    drvdata->used = 0;
    chrdev->drvdata = drvdata;

    DPRINTF("end major:%d", MAJOR(chrdev->dev));
    return 0;

free_long_dev:
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_LONG_MINOR));
free_raw_dev:
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_RAW_MINOR));
free_dev:
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_MINOR));
err:
    pr_err("unable to allocate char dev [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_destroy_chrdev(struct axiomnet_drvdata *drvdata,
        struct axiomnet_chrdev *chrdev)
{
    DPRINTF("start");

    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_RDMA_MINOR));
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_LONG_MINOR));
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_RAW_MINOR));
    device_destroy(chrdev->dclass, MKDEV(MAJOR(chrdev->dev),
                AXIOMNET_DEV_MINOR));
    chrdev->drvdata = NULL;

    DPRINTF("end");
}

static int axiomnet_init_chrdev(struct axiomnet_chrdev *chrdev)
{
    int err = 0;
    DPRINTF("start");

    err = alloc_chrdev_region(&chrdev->dev, AXIOMNET_DEV_MINOR,
                    AXIOMNET_DEV_MAX, AXIOMNET_DEV_NAME);
    if (err) {
        goto err;
    }

    /* axiom device init */
    cdev_init(&chrdev->cdev, &axiomnet_fops);
    err = cdev_add(&chrdev->cdev,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_MINOR), 1);
    if (err < 0) {
        goto free_dev;
    }

    /* axiom raw device init */
    cdev_init(&chrdev->cdev_raw, &axiomnet_raw_fops);
    err = cdev_add(&chrdev->cdev_raw,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_RAW_MINOR), 1);
    if (err < 0) {
        goto free_cdev;
    }

    /* axiom long device init */
    cdev_init(&chrdev->cdev_long, &axiomnet_long_fops);
    err = cdev_add(&chrdev->cdev_long,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_LONG_MINOR), 1);
    if (err < 0) {
        goto free_cdev_raw;
    }

    /* axiom rdma device init */
    cdev_init(&chrdev->cdev_rdma, &axiomnet_rdma_fops);
    err = cdev_add(&chrdev->cdev_rdma,
            MKDEV(MAJOR(chrdev->dev), AXIOMNET_DEV_RDMA_MINOR), 1);
    if (err < 0) {
        goto free_cdev_long;
    }


    chrdev->dclass = class_create(THIS_MODULE, AXIOMNET_DEV_CLASS);
    if (IS_ERR(chrdev->dclass)) {
        err = PTR_ERR(chrdev->dclass);
        goto free_cdev_rdma;
    }

    return 0;

free_cdev_rdma:
    cdev_del(&chrdev->cdev_rdma);
free_cdev_long:
    cdev_del(&chrdev->cdev_long);
free_cdev_raw:
    cdev_del(&chrdev->cdev_raw);
free_cdev:
    cdev_del(&chrdev->cdev);
free_dev:
    unregister_chrdev_region(chrdev->dev, AXIOMNET_DEV_MINOR);
err:
    pr_err("unable to init char dev [error %d]\n", err);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_cleanup_chrdev(struct axiomnet_chrdev *chrdev)
{
    DPRINTF("start");

    class_unregister(chrdev->dclass);
    class_destroy(chrdev->dclass);
    cdev_del(&chrdev->cdev_rdma);
    cdev_del(&chrdev->cdev_long);
    cdev_del(&chrdev->cdev_raw);
    cdev_del(&chrdev->cdev);
    unregister_chrdev_region(chrdev->dev, AXIOMNET_DEV_MINOR);

    DPRINTF("end");
}

/********************** AxiomNet Module [un]init *****************************/

/* Entry point for loading the module */
int axiomnet_init(void)
{
    int err = 0;
    DPRINTF("start");

    err = axiomnet_init_chrdev(&chrdev);

    DPRINTF("end");
    return err;
}

/* Entry point for unloading the module */
void axiomnet_cleanup(void)
{
    DPRINTF("start");
    axiomnet_cleanup_chrdev(&chrdev);
    DPRINTF("end");
}
