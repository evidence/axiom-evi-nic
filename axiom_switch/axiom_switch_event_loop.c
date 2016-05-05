/*!
 * \file axiom_switch_event_loop.c
 *
 * \version     v0.5
 * \date        2016-05-03
 *
 * This file implements the event loop API used in the Axiom Switch
 */
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "axiom_switch.h"
#include "axiom_switch_event_loop.h"

/*! \brief verbose switch (if it's set, print verbose messsagges) */
extern int verbose;

void
axsw_event_loop_init(axsw_event_loop_t *el_status)
{
    /* Initialize the timeout to 3 minutes. */
    el_status->timeout = (3 * 60 * 1000);
    el_status->compress_array = 0;
    el_status->fds_tail = 0;
    el_status->fds_size = AXSW_FDS_SIZE;

    memset(el_status->fds, 0 , sizeof(el_status->fds));
}

int
axsw_event_loop_add_sd(axsw_event_loop_t *el_status, int new_sd, int events)
{
    int cur_tail = el_status->fds_tail;

    if (cur_tail >= el_status->fds_size)
        return -1;

    el_status->fds[cur_tail].fd = new_sd;
    el_status->fds[cur_tail].events = events;
    el_status->fds_tail++;

    return el_status->fds_tail;
}

void
axsw_event_loop_close(axsw_event_loop_t *el_status, int fds_index)
{
    IPRINTF(verbose, "Close connection - sd: %d", el_status->fds[fds_index].fd);
    close(el_status->fds[fds_index].fd);
    el_status->fds[fds_index].fd = -1;
    el_status->compress_array = 1;
}


void
axsw_event_loop_compress(axsw_event_loop_t *el_status)
{
    int i, j;

    if (el_status->compress_array) {
        el_status->compress_array = 0;
        for (i = 0; i < el_status->fds_tail; i++) {
            if (el_status->fds[i].fd == -1) {
                for(j = i; j < el_status->fds_tail; j++) {
                    el_status->fds[j].fd = el_status->fds[j+1].fd;
                }
                i--;
                el_status->fds_tail--;
            }
        }
    }
}

int
axsw_event_loop_get_tail(axsw_event_loop_t *el_status)
{
    return el_status->fds_tail;
}

int
axsw_event_loop_get_revents(axsw_event_loop_t *el_status, int fds_index)
{
    return el_status->fds[fds_index].revents;
}

int
axsw_event_loop_get_fd(axsw_event_loop_t *el_status, int fds_index)
{
    return el_status->fds[fds_index].fd;
}

int
axsw_event_loop_poll(axsw_event_loop_t *el_status)
{
    int ret;

    ret = poll(el_status->fds, el_status->fds_tail, el_status->timeout);
    if (ret < 0) {
        perror("poll() FAILED");
    } else if (ret == 0) {
        IPRINTF(verbose, "  poll() timed out\n");
        ret = 1;
    }

    return ret;
}
