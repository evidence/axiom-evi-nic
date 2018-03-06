/*!
 * \file axiom_netdev_common.h
 *
 * \version     v1.1
 * \date        2017-03-02
 *
 * This file contains the common API for the Axiom NIC kernel module.
 *
 * Copyright (C) 2017, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NETDEV_COMMON_H
#define AXIOM_NETDEV_COMMON_H

struct axiomnet_drvdata;

/*! \brief Allocates AXIOM driver
 *
 *  \param drvdata      AXIOM driver private data pointer
 *  \param dev_api      AXIOM HW API pointer
 *
 *  \return Returns 0 on success, negative on failure
 */
int axiomnet_probe(struct axiomnet_drvdata *drvdata, axiom_dev_t *dev_api);

/*! \brief Remove AXIOM driver
 *
 *  \param drvdata      AXIOM driver private data pointer
 *
 *  \return Returns 0 on success, negative on failure
 */
int axiomnet_remove(struct axiomnet_drvdata *drvdata);

/*! \brief Handle interrupt
 *
 *  \param drvdata      AXIOM driver private data pointer
 */
void axiomnet_irqhandler(struct axiomnet_drvdata *drvdata);

/*! \brief Initialize AXIOM driver character devices
 *
 *  \param drvdata      AXIOM driver private data pointer
 *  \param dev_api      AXIOM HW API pointer
 *
 *  \return Returns 0 on success, negative on failure
 */
int axiomnet_init(void);

/*! \brief Cleanup AXIOM driver character devices
 *
 *  \param drvdata      AXIOM driver private data pointer
 *  \param dev_api      AXIOM HW API pointer
 *
 *  \return Returns 0 on success, negative on failure
 */
void axiomnet_cleanup(void);
#endif /* AXIOM_NETDEV_H */
