#ifndef EVI_QUEUE_h
#define EVI_QUEUE_h

#define EVIQ_NONE       -1

#ifdef __KERNEL__
#define EVI_MALLOC(_1)  (kmalloc(_1, GFP_KERNEL))
#define EVI_FREE(_1)    (kfree(_1))
#define EVI_PRINTF(...)  (printk(KERN_INFO __VA_ARGS__))
#else /* !__KERNEL__ */
#define EVI_MALLOC(_1)  (malloc(_1))
#define EVI_FREE(_1)    (free(_1))
#define EVI_PRINTF(...)  (printf(__VA_ARGS__))
#endif /* __KERNEL__ */

typedef int16_t eviq_pnt_t;

typedef struct evi_queue {
    eviq_pnt_t *head;
    eviq_pnt_t *tail;
    eviq_pnt_t free;
    eviq_pnt_t *next;

    int queues;
    int free_elems;
} evi_queue_t;

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

static int
eviq_init(evi_queue_t *q, int queues, int free_elems)
{
    int i;

    q->head = q->tail = q->next = NULL;
    q->queues = queues;
    q->free_elems = free_elems;

    q->head = EVI_MALLOC(queues * sizeof(*(q->head)));
    if (q->head == NULL)
        goto err;

    q->tail = EVI_MALLOC(queues* sizeof(*(q->tail)));
    if (q->tail == NULL)
        goto err;

    q->next = EVI_MALLOC(free_elems * sizeof(*(q->next)));
    if (q->tail == NULL)
        goto err;


    for (i = 0; i < queues; i++) {
        q->head[i] = EVIQ_NONE;
        q->tail[i] = EVIQ_NONE;
    }

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

inline static int
eviq_free_avail(evi_queue_t *q)
{
    return (q->free != EVIQ_NONE);
}

inline static eviq_pnt_t
eviq_free_head(evi_queue_t *q)
{
    return q->free;
}

inline static eviq_pnt_t
eviq_insert(evi_queue_t *q, int queue_id)
{
    eviq_pnt_t slot, old_tail;

    /* remove slot at the head of the free queue */
    slot = q->free;
    /* no slot to remove */
    if (slot == EVIQ_NONE) {
        return EVIQ_NONE;
    }

    q->free = q->next[slot];

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

    return slot;
}

inline static int
eviq_queue_avail(evi_queue_t *q, int queue_id)
{
    return (q->head[queue_id] != EVIQ_NONE);
}

inline static eviq_pnt_t
eviq_queue_head(evi_queue_t *q, int queue_id)
{
    return q->head[queue_id];
}

inline static eviq_pnt_t
eviq_remove(evi_queue_t *q, int queue_id)
{
    eviq_pnt_t slot, new_head;

    /* remove slot at the head of the queue */
    slot = q->head[queue_id];
    /* no slot to remove */
    if (slot == EVIQ_NONE) {
        return EVIQ_NONE;
    }

    new_head = q->next[slot];

    if (new_head == EVIQ_NONE) {
        /* if the queue is empty, we need update also the tail */
        q->tail[queue_id] = EVIQ_NONE;
    }

    q->head[queue_id] = q->next[slot];

    /* insert slot at the head of the free list */
    q->next[slot] = q->free;
    q->free = slot;

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
