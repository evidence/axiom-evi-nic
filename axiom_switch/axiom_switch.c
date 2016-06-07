/*!
 * \file axiom_switch.c
 *
 * \version     v0.5
 * \date        2016-05-03
 *
 * This file implements the Axiom Switch: emulate network topology to
 * interconnect multiple QEMU AXIOM VMs to each other.
 */
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
#include <getopt.h>
#include <inttypes.h>

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_switch_topology.h"
#include "axiom_switch_event_loop.h"
#include "axiom_switch_qemu.h"

int verbose = 0;

static void
usage(void)
{
    printf("usage: axiom_switch [arguments] -f file_name | -r -n nodes |\n");
    printf("                                -m -n nodes | -m -x cols -y rows\n");
    printf("AXIOM switch: emulate network topology to interconnect multiple\n");
    printf("              QEMU AXIOM VMs to each other.\n\n");
    printf("Arguments:\n");
    printf("-f, --file      file_name   file that contains the topology\n");
    printf("                            The file must have one line per node,\n");
    printf("                            each line (0...N-1) must contain 4 integer,\n");
    printf("                            line_x: IF0 IF1 IF2 IF3\n");
    printf("                            eg. line_2:  3  1  0  255\n");
    printf("                            node_2 is connected on its interface IF0 with node_3,\n");
    printf("                            on IF1 with node_1, on IF2 with node_0\n");
    printf("                            and IF3 is disconnected (255 = NULL_NODE)\n");
    printf("-r, --ring                  create a ring topology \n");
    printf("-n, --nodes     nodes       number of nodes (ring or 2-D mesh)\n");
    printf("-m, --mesh                  create a 2-D torus mesh topology \n");
    printf("-x, --columns   cols        number of columns (2-D mesh)\n");
    printf("-y, --rows      rows        number of rows (2-D mesh)\n");
    printf("-v, --verbose               verbose output\n");
    printf("-h, --help                  print this help\n\n");
}

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



int
main (int argc, char *argv[])
{
    int ret, fds_tail_max_listen, i;
    char filename[100];
    int n, num_ports = 0, end_server = 0;
    int listen_sd[AXSW_PORT_MAX];
    axiom_raw_eth_t axiom_raw_eth_msg;
    axsw_logic_t logic_status;
    axsw_event_loop_t el_status;
    int file_ok = 0, ring_ok = 0, mesh_ok = 0, n_ok = 0;
    int rows_ok = 0, columns_ok = 0;
    uint8_t rows, columns;
    int long_index =0;
    int opt = 0;
    static struct option long_options[] = {
        {"file", required_argument, 0, 'f'},
        {"ring", no_argument, 0, 'r'},
        {"mesh", no_argument, 0, 'm'},
        {"columns", required_argument, 0, 'x'},
        {"rows", required_argument, 0, 'y'},
        {"nodes", required_argument, 0, 'n'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv,"f:rmn:x:y:vh",
                         long_options, &long_index )) != -1) {
        switch (opt) {
            case 'f' :
                if (sscanf(optarg, "%s", filename) != 1) {
                    usage();
                    exit(-1);
                } else {
                    file_ok = 1;
                }
                break;

            case 'r' :
                ring_ok = 1;
                break;

            case 'm' :
                mesh_ok = 1;
                break;

            case 'n' :
                if (sscanf(optarg, "%i", &n) != 1) {
                    usage();
                    exit(-1);
                } else {
                    n_ok = 1;
                }
                break;

            case 'x' :
                if (sscanf(optarg, "%" SCNu8, &columns) != 1) {
                    usage();
                    exit(-1);
                } else {
                    rows_ok = 1;
                }
                break;

            case 'y' :
                if (sscanf(optarg, "%" SCNu8, &rows) != 1) {
                    usage();
                    exit(-1);
                } else {
                    columns_ok = 1;
                }
                break;

            case 'v':
                verbose = 1;
                break;

            case 'h':
            default:
                usage();
                exit(-1);
        }
    }

    axsw_init_topology(&logic_status);

    /* check file presence */
    if (file_ok == 1) {
        /* init the topology structure */
        num_ports = axsw_make_topology_from_file(&logic_status, filename);
        if (num_ports < 0) {
            printf("Error in reading topology from file\n");
            exit(-1);
        }
    } else {
        /* check ring parameter */
        if (ring_ok == 1) {
            if ((rows_ok == 1) || (columns_ok == 1)) {
                printf("Please, for RING topology insert only -n option\n");
                exit (-1);
            }
            /* make ring topology with the inserted nuber of nodes */
            if (n_ok == 1) {
                if ((n < 2) || (n > AXIOM_NODES_MAX)) {
                    printf("Please, for RING topology insert a number of nodes "
                           "between 2 and %d\n",
                            AXIOM_NODES_MAX);
                    exit (-1);
                } else {
                    num_ports = n;
                    /* init the selected topology */
                    axsw_make_ring_topology(&logic_status, n);
                }
            } else {
               usage();
               exit(-1);
            }
        } else if (mesh_ok == 1) {
            /* make mesh topology with the inserted nuber of nodes */
            if ((n_ok == 1) && ((rows_ok == 1) || (columns_ok == 1))) {
                printf("Too many options inserted for MESH topology \n");
                exit(-1);
            }

            /* make mesh topology with the inserted nuber of nodes */
            if (n_ok == 1) {
                if ((n < 4) || (n > AXIOM_NODES_MAX)) {
                    printf("Please, for MESH topology insert a number of nodes "
                           "between 4 and %d\n",
                            AXIOM_NODES_MAX);
                    exit (-1);
                }

                ret = axsw_check_mesh(n, &rows, &columns);
                if (ret == -1){
                    printf ("The inserted number of nodes is a prime number\n");
                    printf ("MESH topology not possible \n");
                    exit (-1);
                }

                num_ports = n;
                /* init the selected topology */
                axsw_make_mesh_topology(&logic_status, rows, columns);

            } else if ((rows_ok == 1) && (columns_ok == 1)) {
                num_ports = rows * columns;
                /* init the selected topology */
                axsw_make_mesh_topology(&logic_status, rows, columns);
            } else {
               usage();
               exit(-1);
            }
        } else {
           usage();
           exit(-1);
        }
    }

    axsw_logic_init(&logic_status, num_ports);
    axsw_print_topology(&logic_status);
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

        IPRINTF(verbose, "Waiting on poll()...");
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
                int new_sd, if_id;

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
                IPRINTF(verbose, "New incoming connection - sd: %d", new_sd);
                ret = axsw_event_loop_add_sd(&el_status, new_sd, POLLIN);
                if (ret < 0) {
                    EPRINTF("no space in fds array");
                    break;
                }

                axsw_logic_set_vm_sd(&logic_status, vm_index, new_sd);

                for (if_id = 0; if_id < AXIOM_INTERFACES_MAX; if_id++) {
                    int connected = 0;
                    if (axsw_logic_find_neighbour_if(&logic_status, vm_index,
                                if_id) >= 0) {
                        connected = 1;
                    }

                    ret = axsw_qemu_send_ifinfo(new_sd, if_id, connected);
                    if (ret < 0) {
                        EPRINTF("send_ifinfo error");
                        break;
                    }
                }

            } else {
                int dst_sd;

                IPRINTF(verbose, "New incoming packet - source sd: %d", fd);
                /* receive ethernet packet */
                ret = axsw_qemu_recv(fd, &axiom_raw_eth_msg);

                if (ret < 0) {
                    axsw_event_loop_close(&el_status, i);
                    axsw_logic_clean_vm_sd(&logic_status, fd);
                    continue;
                } else if (ret == 0) {
                    continue;
                }

                /* forward the received message */
                dst_sd = axsw_logic_forward(&logic_status, fd,
                        &axiom_raw_eth_msg);
                if (dst_sd < 0)
                    continue;

                IPRINTF(verbose, "Packet forwarded - destination sd: %d",
                        dst_sd);
                /* send ethernet packet */
                ret = axsw_qemu_send(dst_sd, &axiom_raw_eth_msg);
                if (ret < 0) {
                    continue;
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
