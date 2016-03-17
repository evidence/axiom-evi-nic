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

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_switch_event_loop.h"
#include "axiom_switch_qemu.h"


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



int main (int argc, char *argv[])
{
    int ret, fds_tail_max_listen, i;
    int num_ports = 0, end_server = 0;
    int listen_sd[AXSW_PORT_MAX];
    axiom_raw_eth_t axiom_raw_eth_msg;
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
        int current_size = axsw_event_loop_get_tail(&el_status);

        DPRINTF("Waiting on poll()...");
        ret = axsw_event_loop_poll(&el_status);
        if (ret < 0) {
            DPRINTF("poll error");
            break;
        }

        for (i = 0; i < current_size; i++) {
            int vm_index, revents, fd;

            revents = axsw_event_loop_get_revents(&el_status, i);

            if (revents == 0)
                continue;

            if (revents != POLLIN) {
                EPRINTF("revents = %d", revents);
                end_server = 1;
                break;
            }

            fd = axsw_event_loop_get_fd(&el_status, i);

            /* check listener socket */
            if (i < fds_tail_max_listen &&
                    listen_socket_find(listen_sd, i, fd, &vm_index)) {
                int new_sd;

                /* Accept each incoming connection */
                new_sd = accept(listen_sd[i], NULL, NULL);
                if (new_sd < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("  accept() failed");
                        end_server = 1;
                    }
                    break;
                }

                /* Add the new incoming connection to the fds structure */
                DPRINTF("New incoming connection - %d", new_sd);
                ret = axsw_event_loop_add_sd(&el_status, new_sd, POLLIN);
                if (ret < 0) {
                    EPRINTF("no space in fds array");
                    break;
                }

                axsw_logic_set_vm_sd(&logic_status, vm_index, new_sd);

            } else {
                int dst_sd;

                /* receive ethernet packet */
                ret = axsw_qemu_recv(fd, &axiom_raw_eth_msg);

                if (ret < 0) {
                    axsw_event_loop_close(&el_status, i);
                } else if (ret == 0) {
                    continue;
                }

                /* forward the received message */
                dst_sd = axsw_logic_forward(&logic_status, fd, &axiom_raw_eth_msg);
                if (dst_sd < 0)
                    continue;

                /* send ethernet packet */
                ret = axsw_qemu_send(dst_sd, &axiom_raw_eth_msg);
                if (ret < 0) {
                    axsw_event_loop_close(&el_status, i);
                }
            }
        }

        axsw_event_loop_compress(&el_status);

    } while (end_server == 0); /* End of serving running.    */

    /* Clean up all of the sockets that are open */
    for (i = 0; i < el_status.fds_tail; i++) {
        if(el_status.fds[i].fd >= 0) {
            close(el_status.fds[i].fd);
        }
    }

    return 0;
}
