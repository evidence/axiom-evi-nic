/*!
 * \file axiom_netdev_sysfs.h
 *
 * \version     v0.15
 * \date        2017-12-21
 *
 * This file contains the implementation of the AXIOM sysfs.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#ifndef AXIOM_NETDEV_SYSFS_H
#define AXIOM_NETDEV_SYSFS_H

/*! \brief AXIOM sysfs data */
struct axiomnet_sysfs {
    struct device *dev;            /*!< \brief root device of sysfs */
    struct kobject *param;         /*!< \brief parameters folder kobject */
    struct kobject *info;          /*!< \brief info folder kobject */
    struct kobject *kthread_raw;   /*!< \brief RAW kthread folder kobject */
    struct kobject *kthread_rdma;  /*!< \brief RDMA kthread folder kobject */
    struct kobject *kthread_wtd;   /*!< \brief WTD kthread folder kobject */

    struct axiomnet_drvdata *drvdata;   /*!< \brief AXIOM driver data */

    /* parameters */
    uint32_t watchdog_period_msec;  /*!< \brief watchdog period in msec */
    uint32_t retry_delay_usec;      /*!< \brief usec to sleep when retry to TX a
                                                long packet */
};

/*!
 * \brief Init the AXIOM kernel sysfs parameters.
 *
 * \param name          AXIOM kernel module name
 * \param axsys         AXIOM sysfs data to init
 * \param drvdata       AXIOM driver data
 *
 * \return 0 on success, an error (< 0) otherwise.
 */
int
axiom_sysfs_init(const char *name, struct axiomnet_sysfs *axsys,
        struct axiomnet_drvdata *drvdata);

/*!
 * \brief Uninit the AXIOM kernel sysfs parameters.
 *
 * \param axsys         AXIOM sysfs data to uninit
 */
void
axiom_sysfs_uninit(struct axiomnet_sysfs *axsys);

#endif /* AXIOM_NETDEV_SYSFS_H */
