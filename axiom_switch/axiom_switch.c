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
#include "axiom_switch_configuration.h"

#define AXSW_BUF_SIZE           1024

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


int main (int argc, char *argv[])
{
    int ret, timeout, end_server = 0, compress_array = 0;
    int nfds = 0, current_size = 0, i, j;
    int num_ports = 0;
    char buffer[AXSW_BUF_SIZE];
    struct pollfd fds[AXSW_PORT_MAX*2];
    int listen_sd[AXSW_PORT_MAX];
    axsw_logic_t logic_status;
    uint8_t vm_index;
    uint32_t axiom_msg_length;


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

    memset(fds, 0 , sizeof(fds));

    axsw_logic_init(&logic_status);

    /* listening sockets creation */
    for (i = 0; i < num_ports; i++) {
        ret = listen_socket_init(&listen_sd[i], AXSW_PORT_START + i);
        if (ret) {
            EPRINTF("listen_socket_init error");
            exit(-1);
        }

        fds[i].fd = listen_sd[i];
        fds[i].events = POLLIN;
        nfds++;
    }

    /* Initialize the timeout to 3 minutes. */
    timeout = (3 * 60 * 1000);

    /* main event loop */
    do {
        DPRINTF("Waiting on poll()...\n");
        ret = poll(fds, nfds, timeout);
        if (ret < 0) {
            perror("  poll() failed");
            break;
        }
        if (ret == 0) {
            IPRINTF("  poll() timed out\n");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            if (fds[i].revents == 0)
                continue;

            if(fds[i].revents != POLLIN) {
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = 1;
                break;
            }

            if (fds[i].fd == listen_sd[i]) {
                int new_sd = -1;

                /* Listening descriptor */
                do {
                    /* Accept each incoming connection */
                    new_sd = accept(listen_sd[i], NULL, NULL);
                    if (new_sd < 0) {
                        if (errno != EWOULDBLOCK) {
                            perror("  accept() failed");
                            end_server = 1;
                        }
                        break;
                    }

                    /* Add the new incoming connection to the */
                    /* fds structure */
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;

                    /* get index of the virtual machine associated with
                        the connected socket (new_sd)*/
                    if (compute_vm_index_from_listen_sd(listen_sd[i], &vm_index) == -1)
                    {
                        printf("Error getting socket port \n");
                        end_server = 1;
                    }
                    else
                    {
                        /* memorize socket associated with the
                         * virtual machine number vm_index */
                        logic_status.vm_sd[vm_index] = new_sd;
                    }

                } while (new_sd != -1);
            } else {
                /* an existing connection must be readable */
                int close_conn = 0;

                do {
                    /* receive the length of the ethernet packet */
                    ret = recv(fds[i].fd, &axiom_msg_length, sizeof(int), MSG_WAITALL);
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
                    if (axiom_msg_length > sizeof(buffer))
                    {
                        printf("Can't receive this too long message\n");
                    }
                    else
                    {
                        /* receive ethernet packet */
                        ret = recv(fds[i].fd, buffer, axiom_msg_length, MSG_WAITALL);

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
                            axsw_logic_forward(&logic_status, buffer, axiom_msg_length,
                                    fds[i].fd);
                        }
                    }

                } while(1);

                if (close_conn == 1) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = 1;
                }
            }
        }

        if (compress_array) {
            compress_array = 0;
            for (i = 0; i < nfds; i++) {
                if (fds[i].fd == -1) {
                    for(j = i; j < nfds; j++) {
                        fds[j].fd = fds[j+1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }

    } while (end_server == 0); /* End of serving running.    */

    /* Clean up all of the sockets that are open */
    for (i = 0; i < nfds; i++) {
        if(fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }

    return 0;
}
