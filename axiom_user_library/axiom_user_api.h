#ifndef AxiomAPI_h
#define AxiomAPI_h

/*
 * This file contains an interface between the ForthAPI and userspace
 * application.
 *
 * 20160229 - v0.1 - Initial version
 */

#include "axiom_nic_api_user.h"


axiom_dev_t *axiom_open(void);
void axiom_close(axiom_dev_t *dev);

#endif /* !AxiomAPI_h */
