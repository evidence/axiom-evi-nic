#ifndef EVI_QUEUE_h
#define EVI_QUEUE_h
/*!
 * \file evi_queue.h
 *
 * \version     v0.8
 * \date        2016-05-03
 *
 * This file contains the EVI queue manager. It can handle multiple queue of
 * filled elements and one queue of free elements.
 */

#define EVIQ_NONE       -1              /*!< \brief none elements */

#ifdef __KERNEL__
#define EVI_MALLOC(_1)  (kmalloc(_1, GFP_KERNEL))
#define EVI_FREE(_1)    (kfree(_1))
#define EVI_PRINTF(...)  (printk(KERN_INFO __VA_ARGS__))
#else /* !__KERNEL__ */
#define EVI_MALLOC(_1)  (malloc(_1))
#define EVI_FREE(_1)    (free(_1))
#define EVI_PRINTF(...)  (printf(__VA_ARGS__))
#endif /* __KERNEL__ */

/*! \brief Pointer used in the EVI queue manager */
typedef int16_t eviq_pnt_t;

/*! \brief EVI queue status */
typedef struct evi_queue {
    int queues;         /*!< number of queue handled */
    int free_elems;     /*!< number of initial free elements */

    eviq_pnt_t *head;   /*!< \brief array of the queue head */
    eviq_pnt_t *tail;   /*!< \brief array of the queue tail */

    eviq_pnt_t free;    /*!< \brief queue of free elems */

    eviq_pnt_t *next;   /*!< \brief array for all elements to chain them */
} evi_queue_t;

/*!
 * \brief Release the resorces for the EVI queue status
 *
 * \param q             EVI queue status pointer
 */
static void
eviq_release(evi_queue_t *q)
{
    if (q->next)
        EVI_FREE(q->next);
    if (q->tail)
        EVI_FREE(q->tail);
    if (q->head)
        EVI_FREE(q->head);

    q->head = q->tail = q->next = NULL;
}

/*!
 * \brief Init the EVI queue status
 *
 * \param q             EVI queue status pointer
 * \param queues        Number of queues
 * \param free_elems    Number of initial free elements
 *
 * \return 0 on success, otherwise -1
 */
static int
eviq_init(evi_queue_t *q, int queues, int free_elems)
{
    int i;

    q->head = q->tail = q->next = NULL;
    q->queues = queues;
    q->free_elems = free_elems;

    if (queues > 0) {
        q->head = EVI_MALLOC(queues * sizeof(*(q->head)));
        if (q->head == NULL)
            goto err;

        q->tail = EVI_MALLOC(queues* sizeof(*(q->tail)));
        if (q->tail == NULL)
            goto err;

        for (i = 0; i < queues; i++) {
            q->head[i] = EVIQ_NONE;
            q->tail[i] = EVIQ_NONE;
        }
    }

    q->next = EVI_MALLOC(free_elems * sizeof(*(q->next)));
    if (q->next == NULL)
        goto err;

    q->free = 0;

    for (i = 0; i < (free_elems - 1); i++) {
        q->next[i] = i + 1;
    }
    q->next[i] = EVIQ_NONE;

    return 0;

err:
    eviq_release(q);
    return -1;
}

/*!
 * \brief Chek if there are free elements in the free queue
 *
 * \param q             EVI queue status pointer
 *
 * \return 0 if there are not space, otherwise an integer != 0
 */
inline static int
eviq_free_avail(evi_queue_t *q)
{
    return (q->free != EVIQ_NONE);
}

/*!
 * \brief Pop one slot from the free queue
 *
 * \param q             EVI queue status pointer
 *
 * \return slot from the head
 */
inline static eviq_pnt_t
eviq_free_pop(evi_queue_t *q)
{
    eviq_pnt_t slot;

    /* remove slot at the head of the free queue */
    slot = q->free;
    /* no slot to remove */
    if (unlikely(slot == EVIQ_NONE)) {
        return EVIQ_NONE;
    }

    q->free = q->next[slot];

    return slot;
}

/*!
 * \brief Push one slot in the free queue
 *
 * \param q             EVI queue status pointer
 * \param slot          Slot to enqueue
 */
inline static void
eviq_free_push(evi_queue_t *q, eviq_pnt_t slot)
{
    /* insert slot at the head of the free list */
    q->next[slot] = q->free;
    q->free = slot;
}

/*!
 * \brief Chek if there are elements in the specified queue
 *
 * \param q             EVI queue status pointer
 * \param queue_id      Queue identifier
 *
 * \return 0 if there are not elements available, otherwise an integer != 0
 */
inline static int
eviq_avail(evi_queue_t *q, int queue_id)
{
    if (unlikely(q->queues == 0))
        return 0;

    return (q->head[queue_id] != EVIQ_NONE);
}

/*!
 * \brief Insert one element at the tail of the specified queue
 *
 * \param q             EVI queue status pointer
 * \param queue_id      Queue identifier
 * \param slot          Slot to insert
 *
 */
inline static void
eviq_enqueue(evi_queue_t *q, int queue_id, eviq_pnt_t slot)
{
    eviq_pnt_t old_tail;

    if (unlikely(q->queues == 0))
        return;

    /* insert at the tail */
    q->next[slot] = EVIQ_NONE;
    old_tail = q->tail[queue_id];

    if (old_tail != EVIQ_NONE) {
        q->next[old_tail] = slot;
    } else {
        /* if the queue is empty, we need update also the head */
        q->head[queue_id] = slot;
    }

    q->tail[queue_id] = slot;
}

/*!
 * \brief Remove one element at the head from the specified queue
 *
 * \param q             EVI queue status pointer
 * \param queue_id      Queue identifier
 *
 * \return eviq_pnt_t to the slot removed on success, otherwise -1
 */
inline static eviq_pnt_t
eviq_dequeue(evi_queue_t *q, int queue_id)
{
    eviq_pnt_t slot, new_head;

    if (unlikely(q->queues == 0))
        return EVIQ_NONE;

    /* remove slot at the head of the queue */
    slot = q->head[queue_id];
    /* no slot to remove */
    if (unlikely(slot == EVIQ_NONE)) {
        return EVIQ_NONE;
    }

    new_head = q->next[slot];

    if (new_head == EVIQ_NONE) {
        /* if the queue is empty, we need update also the tail */
        q->tail[queue_id] = EVIQ_NONE;
    }

    q->head[queue_id] = q->next[slot];

    return slot;
}

#ifdef EVIQ_DEBUG
static void
_eviq_print_queue(evi_queue_t *q, eviq_pnt_t slot)
{
    while(1) {

        EVI_PRINTF("%d ", slot);

        if (slot == EVIQ_NONE)
            break;

        slot = q->next[slot];
    }
    EVI_PRINTF("\n");
}

static void
eviq_print_queue(evi_queue_t *q, int queue_id)
{
    eviq_pnt_t slot = q->head[queue_id];

    EVI_PRINTF("queue[%d] - head: %d tail: %d\n", queue_id, q->head[queue_id],
            q->tail[queue_id]);
    _eviq_print_queue(q, slot);
}

static void
eviq_print_free(evi_queue_t *q)
{
    eviq_pnt_t slot = q->free;

    EVI_PRINTF("free - head: %d\n", q->free);
    _eviq_print_queue(q, slot);
}
#endif /* EVIQ_DEBUG */

#endif /* EVI_QUEUE_h */
