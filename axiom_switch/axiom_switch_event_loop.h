#ifndef AXIOM_SWITCH_EVENT_LOOP_h
#define AXIOM_SWITCH_EVENT_LOOP_h
/*!
 * \file axiom_switch_event_loop.h
 *
 * \version     v0.7
 * \date        2016-05-03
 *
 * This file contains API to implements the event loop in the Axiom Switch
 */


/*!
 * \brief event loop status
 */
typedef struct axsw_event_loop {
    int timeout;
    int end_server;
    int compress_array;
    int fds_tail;
    int fds_size;
    struct pollfd fds[AXSW_FDS_SIZE];
} axsw_event_loop_t;

/*!
 * \brief Init the event loop status
 *
 * \param el_status     event loop status pointer
 */
void axsw_event_loop_init(axsw_event_loop_t *el_status);

/*!
 * \brief Add new socket descriptor in the event loop
 *
 * \param el_status     event loop status pointer
 * \param new_sd        socket descriptor to add
 * \param events        requested events
 *
 * \return index in the pollfd array
 */
int axsw_event_loop_add_sd(axsw_event_loop_t *el_status, int new_sd, int events);

/*!
 * \brief Close a socket descriptor with a specified index in the pollfd array
 *
 * \param el_status     event loop status pointer
 * \param fds_index     index in the pollfd array
 */
void axsw_event_loop_close(axsw_event_loop_t *el_status, int fds_index);

/*!
 * \brief Compress the pollfd array
 *
 * \param el_status     event loop status pointer
 */
void axsw_event_loop_compress(axsw_event_loop_t *el_status);

/*!
 * \brief Get the current tail in the pollfd array
 *
 * \param el_status     event loop status pointer
 *
 * \return tail value
 */
int axsw_event_loop_get_tail(axsw_event_loop_t *el_status);

/*!
 * \brief Get the requested events of a specified pollfd element
 *
 * \param el_status     event loop status pointer
 * \param fds_index     index in the pollfd array
 *
 * \return registered events
 */
int axsw_event_loop_get_revents(axsw_event_loop_t *el_status, int fds_index);

/*!
 * \brief Get the file descriptor of a specified pollfd element
 *
 * \param el_status     event loop status pointer
 * \param fds_index     index in the pollfd array
 *
 * \return file descriptor
 */
int axsw_event_loop_get_fd(axsw_event_loop_t *el_status, int fds_index);

/*!
 * \brief Perform a poll() with a pollfd array
 *
 * \param el_status     event loop status pointer
 *
 * \return poll return value (number of fd ready)
 */
int axsw_event_loop_poll(axsw_event_loop_t *el_status);

#endif /* !AXIOM_SWITCH_EVENT_LOOP_h */
