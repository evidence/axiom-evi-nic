#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PDEBUG
#include "dprintf.h"

#include "axiom_switch_logic.h"
#include "axiom_switch_topology.h"

#define AXSW_BUF_SIZE           1024
#define AXSW_FDS_SIZE           AXSW_PORT_MAX*2


static int
listen_socket_init(int *listen_sd, uint16_t port) {
    struct sockaddr_in addr;
    int ret, on = 1;

    *listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (*listen_sd < 0) {
        perror("socket() failed");
        return -1;
    }

    ret = setsockopt(*listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on,
            sizeof(on));
    if (ret < 0) {
        perror("setsockopt() failed");
        goto err;
    }

    ret = ioctl(*listen_sd, FIONBIO, (char *)&on);
    if (ret < 0)
    {
        perror("ioctl() failed");
        goto err;
    }

    /* Bind to an incremental ports */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    ret = bind(*listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind() failed");
        goto err;
    }

    ret = listen(*listen_sd, 32);
    if (ret < 0) {
        perror("listen() failed");
        goto err;
    }

    return 0;

err:
    close(*listen_sd);
    return ret;
}

static int
listen_socket_find(int *listen_sd, int fds_index, int sd, int *vm_index)
{
    int i;

    /* listener socket is added in the first slot of fds */
    if (fds_index < AXSW_PORT_MAX && listen_sd[fds_index] == sd) {
        *vm_index = fds_index;
        return 1;
    }


    for(i = 0; i < AXSW_PORT_MAX; i++) {
        if (listen_sd[i] == sd) {
            *vm_index = i;
            return 1;
        }
    }

    return 0;
}

typedef struct axsw_event_loop {
    int timeout;
    int end_server;
    int compress_array;
    int fds_tail;
    int fds_size;
    char buffer[AXSW_BUF_SIZE];
    struct pollfd fds[AXSW_FDS_SIZE];
} axsw_event_loop_t;

static void
axsw_event_loop_init(axsw_event_loop_t *el_status)
{
    /* Initialize the timeout to 3 minutes. */
    el_status->timeout = (3 * 60 * 1000);
    el_status->end_server = 0;
    el_status->compress_array = 0;
    el_status->fds_tail = 0;
    el_status->fds_size = AXSW_FDS_SIZE;

    memset(el_status->fds, 0 , sizeof(el_status->fds));
}

static int
axsw_event_loop_add_sd(axsw_event_loop_t *el_status, int new_sd, int events)
{
    int cur_tail = el_status->fds_tail;

    if (cur_tail >= el_status->fds_size)
        return -1;

    el_status->fds[cur_tail].fd = new_sd;
    el_status->fds[cur_tail].events = events;
    el_status->fds_tail++;

    return cur_tail;
}



int main (int argc, char *argv[])
{
    int ret, fds_tail_max_listen, i, j;
    int num_ports = 0;
    int listen_sd[AXSW_PORT_MAX];
    uint32_t axiom_msg_length;
    axsw_logic_t logic_status;
    axsw_event_loop_t el_status;


    /* first parameter: number of ports */
    if (argc < 2) {
        printf("Parameter required: number of ports\n");
        exit(-1);
    }

    if (sscanf(argv[1], "%i", &num_ports) != 1) {
        perror("parameter is not an integer");
        exit(-1);
    }

    if (num_ports > AXSW_PORT_MAX) {
        printf("Max ports supported is %d\n", AXSW_PORT_MAX);
        exit(-1);
    }

    axsw_logic_init(&logic_status);
    axsw_event_loop_init(&el_status);

    /* listening sockets creation */
    for (i = 0; i < num_ports; i++) {
        ret = listen_socket_init(&listen_sd[i], AXSW_PORT_START + i);
        if (ret) {
            EPRINTF("listen_socket_init error");
            exit(-1);
        }

        ret = axsw_event_loop_add_sd(&el_status, listen_sd[i], POLLIN);
        if (ret < 0) {
            EPRINTF("no space in fds array");
            exit(-1);
        }
        fds_tail_max_listen = ret;
    }


    /* main event loop */
    do {
        int current_size = el_status.fds_tail;

        DPRINTF("Waiting on poll()...\n");
        ret = poll(el_status.fds, el_status.fds_tail, el_status.timeout);
        if (ret < 0) {
            perror("  poll() failed");
            break;
        }
        if (ret == 0) {
            IPRINTF("  poll() timed out\n");
            break;
        }

        for (i = 0; i < current_size; i++) {
            int vm_index;

            if (el_status.fds[i].revents == 0)
                continue;

            if(el_status.fds[i].revents != POLLIN) {
                printf("  Error! revents = %d\n", el_status.fds[i].revents);
                el_status.end_server = 1;
                break;
            }

            /* check listener socket */
            if (i < fds_tail_max_listen &&
                    listen_socket_find(listen_sd, i, el_status.fds[i].fd, &vm_index)) {
                int new_sd;

                /* Accept each incoming connection */
                new_sd = accept(listen_sd[i], NULL, NULL);
                if (new_sd < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("  accept() failed");
                        el_status.end_server = 1;
                    }
                    break;
                }

                /* Add the new incoming connection to the fds structure */
                printf("  New incoming connection - %d\n", new_sd);
                ret = axsw_event_loop_add_sd(&el_status, new_sd, POLLIN);
                if (ret < 0) {
                    EPRINTF("no space in fds array");
                    break;
                }

                axsw_logic_set_vm_sd(&logic_status, vm_index, new_sd);

            } else {
                /* an existing connection must be readable */
                int close_conn = 0;

                do {
                    /* receive the length of the ethernet packet */
                    ret = recv(el_status.fds[i].fd, &axiom_msg_length, sizeof(int), MSG_WAITALL);
                    if (ret < 0) {
                        if (errno != EWOULDBLOCK) {
                            perror("  recv() failed");
                            close_conn = 1;
                        }
                        break;
                    }

                    if (ret == 0) {
                        printf("  Connection closed\n");
                        close_conn = 1;
                        break;
                    }

                    axiom_msg_length = ntohl(axiom_msg_length);
                    if (axiom_msg_length > sizeof(el_status.buffer))
                    {
                        printf("Can't receive this too long message\n");
                    }
                    else
                    {
                        /* receive ethernet packet */
                        ret = recv(el_status.fds[i].fd, el_status.buffer, axiom_msg_length, MSG_WAITALL);

                        if (ret < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("  recv() failed");
                                close_conn = 1;
                            }
                            break;
                        }

                        if (ret == 0) {
                            printf("  Connection closed\n");
                            close_conn = 1;
                            break;
                        }

                        if (ret != axiom_msg_length) {
                            EPRINTF("Received a ethernet packet with unexpected length");
                        } else {
                            /* Manage the received message */
                            axsw_logic_forward(&logic_status, el_status.buffer,
                                    axiom_msg_length, el_status.fds[i].fd);
                        }
                    }

                } while(1);

                if (close_conn == 1) {
                    close(el_status.fds[i].fd);
                    el_status.fds[i].fd = -1;
                    el_status.compress_array = 1;
                }
            }
        }

        if (el_status.compress_array) {
            el_status.compress_array = 0;
            for (i = 0; i < el_status.fds_tail; i++) {
                if (el_status.fds[i].fd == -1) {
                    for(j = i; j < el_status.fds_tail; j++) {
                        el_status.fds[j].fd = el_status.fds[j+1].fd;
                    }
                    i--;
                    el_status.fds_tail--;
                }
            }
        }

    } while (el_status.end_server == 0); /* End of serving running.    */

    /* Clean up all of the sockets that are open */
    for (i = 0; i < el_status.fds_tail; i++) {
        if(el_status.fds[i].fd >= 0) {
            close(el_status.fds[i].fd);
        }
    }

    return 0;
}
