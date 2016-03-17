#ifndef AXIOM_SWITCH_EVENT_LOOP_h
#define AXIOM_SWITCH_EVENT_LOOP_h


typedef struct axsw_event_loop {
    int timeout;
    int end_server;
    int compress_array;
    int fds_tail;
    int fds_size;
    struct pollfd fds[AXSW_FDS_SIZE];
} axsw_event_loop_t;

void axsw_event_loop_init(axsw_event_loop_t *el_status);
int axsw_event_loop_add_sd(axsw_event_loop_t *el_status, int new_sd, int events);
void axsw_event_loop_close(axsw_event_loop_t *el_status, int fds_index);
void axsw_event_loop_compress(axsw_event_loop_t *el_status);
int axsw_event_loop_get_tail(axsw_event_loop_t *el_status);
int axsw_event_loop_get_revents(axsw_event_loop_t *el_status, int fds_index);
int axsw_event_loop_get_fd(axsw_event_loop_t *el_status, int fds_index);
int axsw_event_loop_poll(axsw_event_loop_t *el_status);

#endif /* !AXIOM_SWITCH_EVENT_LOOP_h */
