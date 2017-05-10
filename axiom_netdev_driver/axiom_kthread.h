/*!
 * \file axiom_kthread.h
 *
 * \version     v0.12
 * \date        2016-06-15
 *
 * This file contains the structures and macros for the Axiom kernel kthread.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_KTHREAD_H
#define AXIOM_KTHREAD_H

/*! \brief AXIOM kernel thread worker function pointer */
typedef void (*axkt_worker_fn_t)(void *data);
/*! \brief AXIOM kernel thread work todo function pointer */
typedef bool (*axkt_work_todo_fn_t)(void *data);

/*! \brief AXIOM kernel thread data */
struct axiom_kthread {
    struct task_struct *task;           /*!< \brief Kernel thread task struct */
    wait_queue_head_t wq;               /*!< \brief Kernel thread wait queue */
    atomic_t scheduled;                 /*!< \brief Pending wake_up request */

    axkt_worker_fn_t worker_fn;         /*!< \brief Worker function to exec */
    axkt_work_todo_fn_t work_todo_fn;   /*!< \brief Work todo function to call
                                          to check if there is work todo */
    void *worker_data;                  /*!< \brief Worker private data */
};


/*!
 * \brief Init and start an AXIOM kernel thread.
 *
 * \param ctx           AXIOM kernel thread data context
 * \param worker_fn     Worker function to exec
 * \param work_todo_fn  Work todo function to call to check if there is work
 *                      todo
 * \param worker_data   Worker private data
 * \param name          AXIOM kernel thread name
 *
 * \return 0 on success, an error (< 0) otherwise.
 */
int
axiom_kthread_init(struct axiom_kthread *ctx, axkt_worker_fn_t worker_fn,
        axkt_work_todo_fn_t work_todo_fn, void *worker_data, char *name);

/*!
 * \brief Stop and uninit an AXIOM kernel thread.
 *
 * \param ctx           AXIOM kernel thread data context
 */
void
axiom_kthread_uninit(struct axiom_kthread *ctx);

/*!
 * \brief Wake up an AXIOM kernel thread.
 *
 * \param ctx           AXIOM kernel thread data context
 */
void
axiom_kthread_wakeup(struct axiom_kthread *ctx);

#endif /* AXIOM_KTHREAD_H */
