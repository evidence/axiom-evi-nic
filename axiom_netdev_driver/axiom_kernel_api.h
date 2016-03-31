#ifndef AXIOM_KERNEL_API_H
#define AXIOM_KERNEL_API_H

#include "dprintf.h"
#include "axiom_nic_api_hw.h"


axiom_dev_t *axiom_hw_dev_alloc(void *vregs);
void axiom_hw_dev_free(axiom_dev_t *dev);


/* debug functions */

void axiom_print_status_reg(axiom_dev_t *dev);
void axiom_print_control_reg(axiom_dev_t *dev);
void axiom_print_routing_reg(axiom_dev_t *dev);
void axiom_print_small_queue_reg(axiom_dev_t *dev);

#endif /* !AXIOM_KERNEL_API_H */
