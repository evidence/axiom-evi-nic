/*!
 * \file axiom_kthread.c
 *
 * \version     v0.6
 * \date        2016-06-15
 *
 * This file contains the implementation of the Axiom kernel kthread.
 */
#include <linux/kthread.h>

#include "axiom_kthread.h"
#include "dprintf.h"

inline static bool
axkt_should_stop(struct axiom_kthread *ctx)
{
    return kthread_should_stop();
}

static int
axkt_worker(void *data)
{
    struct axiom_kthread *ctx = data;

    for (;;) {
        wait_event_interruptible(ctx->wq,
                ctx->work_todo_fn(ctx->worker_data) ||
                axkt_should_stop(ctx));

        if (axkt_should_stop(ctx))
            break;

        /* execute the worker function */
        ctx->worker_fn(ctx->worker_data);

        cond_resched();
    }

    return 0;
}

void
axiom_kthread_wakeup(struct axiom_kthread *ctx)
{
    wake_up(&ctx->wq);
}

int
axiom_kthread_init(struct axiom_kthread *ctx, axkt_worker_fn_t worker_fn,
        axkt_work_todo_fn_t work_todo_fn, void *worker_data, char *name)
{
    struct task_struct *task;
    int ret = 0;

    init_waitqueue_head(&ctx->wq);

    task = kthread_create(axkt_worker, (void *) ctx, "axiom-kthread-%s", name);
    if (IS_ERR(task)) {
        EPRINTF("Unable to allocate axiom-kthread");
        ret = PTR_ERR(task);
        goto err;
    }

    /* fill the worker context */
    ctx->task = task;
    ctx->worker_fn = worker_fn;
    ctx->work_todo_fn = work_todo_fn;
    ctx->worker_data = worker_data;

    /* start the kthread */
    wake_up_process(ctx->task);

err:
    return ret;
}

void
axiom_kthread_uninit(struct axiom_kthread *ctx)
{
    if(ctx->task) {
        kthread_stop(ctx->task);
        ctx->task = NULL;
    }
}
