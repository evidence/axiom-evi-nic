#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#include "axiom_switch_logic.h"

#define AXSW_PORT_START         33300
#define AXSW_NUM_NODES          16


int main (int argc, char *argv[])
{
    int    ret, on = 1;
    int    listen_sd[AXSW_NUM_NODES], new_sd = -1;
    int    end_server = 0, compress_array = 0;
    int    close_conn;
    char   buffer[1024];
    struct sockaddr_in addr;
    int    timeout;
    struct pollfd fds[AXSW_NUM_NODES*2];
    int    nfds = 0, current_size = 0, i, j;
    int    port_index, num_ports = 0;

    /* get the number of nodes */
    if (argc < 2) {
        perror("Please, insert the number of node");
        exit(-1);
    }

    if (sscanf (argv[1], "%i", &num_ports) != 1) {
        perror("parameter is not an integer");
        exit(-1);
    }

    memset(fds, 0 , sizeof(fds));

    /* listening sockets creation */
    for (i = 0; i < num_ports; i++) {

        listen_sd[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sd[i] < 0) {
            perror("socket() failed");
            exit(-1);
        }

        ret = setsockopt(listen_sd[i], SOL_SOCKET,  SO_REUSEADDR, (char *)&on,
                sizeof(on));

        if (ret < 0) {
            perror("setsockopt() failed");
            close(listen_sd[i]);
            exit(-1);
        }

        ret = ioctl(listen_sd[i], FIONBIO, (char *)&on);
        if (ret < 0)
        {
            perror("ioctl() failed");
            close(listen_sd[i]);
            exit(-1);
        }

        /* Bind to an incremental address */
        memset(&addr, 0, sizeof(addr));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(AXSW_PORT_START + i);

        ret = bind(listen_sd[i], (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0) {
            perror("bind() failed");
            close(listen_sd[i]);
            exit(-1);
        }

        ret = listen(listen_sd[i], 32);
        if (ret < 0) {
            perror("listen() failed");
            close(listen_sd[i]);
            exit(-1);
        }

        /* Set up the initial listening socket */
        fds[i].fd = listen_sd[i];
        fds[i].events = POLLIN;
        nfds++;
    }

    /* Initialize the timeout to 3 minutes. */
    timeout = (3 * 60 * 1000);

    /*************************************************************/
    /* Loop waiting for incoming connects or for incoming data   */
    /* on any of the connected sockets.                          */
    /*************************************************************/
    do {
        printf("Waiting on poll()...\n");
        ret = poll(fds, nfds, timeout);
        if (ret < 0) {
            perror("  poll() failed");
            break;
        }
        if (ret == 0) {
            printf("  poll() timed out. End program.\n");
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
                } while (new_sd != -1);
            } else {
                /* an existing connection must be readable */
                close_conn = 0;

                do {
                    ret = recv(fds[i].fd, buffer, sizeof(buffer), 0);
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

                    /* Data was received */
                    receive_axiom_msg(buffer, ret);

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
}
