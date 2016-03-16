#ifndef AXIOM_SWITCH_LOGIC_h
#define AXIOM_SWITCH_LOGIC_h

#define AXSW_PORT_MAX           16      /* max port supported */
#define AXSW_PORT_START         33300   /* first port to listen */

/* This function manages the axiom messages*/
int manage_axiom_msg(char *buffer, uint32_t length, int my_sd, int *vm_sd,
        int *node_sd);


#endif /* AXIOM_SWITCH_LOGIC_h */
