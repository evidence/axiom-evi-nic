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

#include "axiom_switch.h"
#include "axiom_switch_packets.h"
#include "axiom_switch_logic.h"
#include "axiom_switch_event_loop.h"
#include "axiom_switch_qemu.h"

extern axiom_topology_t start_topology;

static void usage(void)
{
    printf("usage: ./axiom_switch [[-t port] [-n] [-h]] -t topology -n number of topology or number of nodes \n\n");
    printf("-t, --toplogy   topology_numer      0:pre-existent topology 1:RING \n");
    printf("-n,             number              number of pre-existent topology | number of nodes for other topologies\n");
    printf("-h, --help                          print this help\n");
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



int main (int argc, char *argv[])
{
    int ret, fds_tail_max_listen, i;
    int n, num_ports = 0, topology = 0, end_server = 0;
    int listen_sd[AXSW_PORT_MAX];
    axiom_small_eth_t axiom_small_eth_msg;
    axsw_logic_t logic_status;
    axsw_event_loop_t el_status;
    int topology_ok = 0, n_ok = 0;
    int long_index =0;
    int opt = 0;
    static struct option long_options[] = {
        {"n", required_argument, 0, 'n'},
        {"topology", required_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    axsw_sim_topology_t sim_toplogy;

    while ((opt = getopt_long(argc, argv,"t:n:h",
                         long_options, &long_index )) != -1) {
        switch (opt) {
            case 't' :
                if (sscanf(optarg, "%i", &topology) != 1) {
                    usage();
                    exit(-1);
                }
                else {
                    topology_ok = 1;
                }
                break;

            case 'n' :
                if (sscanf(optarg, "%i", &n) != 1) {
                    usage();
                    exit(-1);
                }
                else {
                    n_ok = 1;
                }
                break;


            case 'h':
            default:
                usage();
                exit(-1);
        }
    }

    /* chack topology paramenter */
    if (topology_ok == 1) {
        switch (topology) {
            case AXTP_DEFAULT_SIM:
                /* pre-esistent toplogy management */
                /* Initialization of pointer to the all
                 * possible topologyies management functions */
                axsw_init_f_topology(&sim_toplogy);

                if (n_ok == 1) {
                    if ((n < 0) || (n > AXTP_NUM_SIM)) {
                        printf("Please, for topology %d insert a simulation number between 0 and %d\n",
                            AXTP_DEFAULT_SIM,
                            AXTP_NUM_SIM-1);
                        exit (-1);
                    }
                    else {
                        num_ports = sim_toplogy.needed_switch_port[n];
                        /* init the selected topology */
                        axsw_init_topology(&start_topology);
                        sim_toplogy.axsw_f_init_topology[n](&start_topology);
                    }
                }
                else {
                   usage();
                   exit(-1);
                }
                break;

            default:
                printf("Topology type not existent!\n");
                exit (-1);
        }
    }
    else {
       usage();
       exit(-1);
    }


#if 0
    /* check toplogy parameter */
    if (topology_ok == 1) {
        if (topology > AXTP_MAX_NUM_SIM) {
            printf("Actual number of supported topology is %d",
                    AXTP_NUM_SIM);
            exit (-1);
        }
        else {
            /* Initialization of pointer to the all
             * possible topologyies management functions */
            axsw_init_f_topology (sim_toplogy.axsw_f_init_topology);

            if (sim_toplogy.needed_switch_port[topology] != num_ports) {
                printf("Number %d",
                        AXTP_NUM_SIM);
                exit (-1);
            }
            else {
                /* init the selected topology */
                axsw_init_topology(&start_topology);
                axsw_f_init_topology[topology](&start_topology);
            }

        }
    }
    else {
        usage();
        exit(-1);
    }
#endif


#if 0
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
#endif
    axsw_logic_init(&logic_status);
    axsw_if_topology_init(&start_topology, num_ports);
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
                IPRINTF("New incoming connection - sd: %d", new_sd);
                ret = axsw_event_loop_add_sd(&el_status, new_sd, POLLIN);
                if (ret < 0) {
                    EPRINTF("no space in fds array");
                    break;
                }

                axsw_logic_set_vm_sd(&logic_status, vm_index, new_sd);

            } else {
                int dst_sd;

                /* receive ethernet packet */
                ret = axsw_qemu_recv(fd, &axiom_small_eth_msg);

                if (ret < 0) {
                    axsw_event_loop_close(&el_status, i);
                    axsw_logic_clean_vm_sd(&logic_status, fd);
                    continue;
                } else if (ret == 0) {
                    continue;
                }

                /* forward the received message */
                dst_sd = axsw_logic_forward(&logic_status, fd, &axiom_small_eth_msg);
                if (dst_sd < 0)
                    continue;

                /* send ethernet packet */
                ret = axsw_qemu_send(dst_sd, &axiom_small_eth_msg);
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
