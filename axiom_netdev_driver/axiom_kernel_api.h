#ifndef AXIOM_KERNEL_API_H
#define AXIOM_KERNEL_API_H

#include "axiom_nic_api.h"

axiom_dev_t *axiom_init_dev(void *vregs);
void axiom_free_dev(axiom_dev_t *dev);


/* debug functions */

void axiom_print_status_reg(axiom_dev_t *dev);
void axiom_print_control_reg(axiom_dev_t *dev);
void axiom_print_routing_reg(axiom_dev_t *dev);
void axiom_print_raw_queue_reg(axiom_dev_t *dev);

#endif /* !AXIOM_KERNEL_API_H */
