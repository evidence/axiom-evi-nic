/*!
 * \file axiom_netdev_x86.c
 *
 * \version     v1.1
 * \date        2017-03-02
 *
 * This file contains the implementation of the Axiom NIC kernel module for x86
 * architecture.
 *
 * Copyright (C) 2017, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include <linux/pci.h>

#include "axiom_netdev.h"

#define AXIOM_PCI_VENDOR_ID     0x0000 /* TODO: define it */
#define AXIOM_PCI_DEVICE_ID     0x0000 /* TODO: define it */

/*! \brief AXIOM x86 device driver data */
struct axiomnet_x86data {
    struct axiomnet_drvdata drvdata;    /*!< \brief AXIOM device driver data */
    axiom_dev_t *dev_api;               /*!< \brief AXIOM dev HW API*/

    struct pci_dev *pdev;               /*!< \brief parent device */
};

/*
 * PCI Device ID Match Table
 * list of (VendorID,DeviceID) supported by this driver
 */
static const struct pci_device_id axiomnet_x86_match[] = {
	{ PCI_DEVICE(AXIOM_PCI_VENDOR_ID, AXIOM_PCI_DEVICE_ID), },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, axiomnet_x86_match);

static irqreturn_t axiomnet_x86_irq(int irq, void *dev_id)
{
    struct axiomnet_x86data *x86data = dev_id;
    irqreturn_t serviced = IRQ_NONE;

    axiomnet_irqhandler(&x86data->drvdata);

    serviced = IRQ_HANDLED;

    return serviced;
}

static int axiomnet_x86_probe(struct pci_dev *pdev,
        const struct pci_device_id *id)
{
    struct axiomnet_x86data *x86data;
    int err = 0;

    /* allocate our structure and fill it out */
    x86data = kzalloc(sizeof(*x86data), GFP_KERNEL);
    if (x86data == NULL)
        return -ENOMEM;

    x86data->pdev = pdev;
    pci_set_drvdata(pdev, x86data);

    /* TODO: map device registers */

    /* TODO: allocate axiom api, maybe we need to change something in the
     *       axiom_hw API
     */
    x86data->dev_api = axiom_hw_dev_alloc(0/* TODO: virtual address of regs */);
    if (x86data->dev_api == NULL) {
        err = -ENOMEM;
        goto free_local;
    }

    axiom_hw_disable_irq(x86data->dev_api);

    /* TODO: setup IRQ */


    /* probe AXIOM common driver */
    err = axiomnet_probe(&x86data->drvdata, x86data->dev_api);
    if (err) {
        goto free_irq;
    }

    IPRINTF(1, "AXIOM NIC driver loaded");
    return 0;

free_irq:
free_local:
    kfree(x86data);
    DPRINTF("error: %d", err);
    return err;
}

static void axiomnet_x86_remove(struct pci_dev *pdev)
{
    struct axiomnet_x86data *x86data = pci_get_drvdata(pdev);

    /* remove AXIOM common driver */
    axiomnet_remove(&x86data->drvdata);

    /* TODO: free IRQ */

    axiom_hw_dev_free(x86data->dev_api);
    kfree(x86data);

    IPRINTF(1, "AXIOM NIC driver unloaded");
}

/*
 * pci driver information
 */
static struct pci_driver axiomnet_x86_driver = {
    .name       = "axiom_net",
    .id_table   = axiomnet_x86_match,
    .probe      = axiomnet_x86_probe,
    .remove     = axiomnet_x86_remove,
};

/********************** AxiomNet Module [un]init *****************************/

/*
 * Entry point for loading the module
 *
 * Returns 0 on success, negative on failure
 */
static int __init axiomnet_x86_init(void)
{
    int err;

    /* init the AXIOM module */
    err = axiomnet_init();
    if (err) {
        goto err;
    }

    /* register pci driver */
    err = pci_register_driver(&axiomnet_x86_driver);
    if (err) {
        goto cleanup;
    }

    return 0;

cleanup:
    axiomnet_cleanup();
err:
    pr_err("unable to init axiomnet module [error %d]\n", err);
    return err;
}

/* Entry point for unloading the module */
static void __exit axiomnet_x86_cleanup(void)
{
    /* unregister pci driver */
    pci_unregister_driver(&axiomnet_x86_driver);

    /* clean the axiom module */
    axiomnet_cleanup();
}

module_init(axiomnet_x86_init);
module_exit(axiomnet_x86_cleanup);
