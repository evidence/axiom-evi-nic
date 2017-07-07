/*!
 * \file axiom_netdev_arm.c
 *
 * \version     v0.11
 * \date        2016-05-03
 *
 * This file contains the implementation of the Axiom NIC kernel module for ARM
 * architecture.
 *
 * Copyright (C) 2017, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include <linux/of.h>
#include <linux/of_device.h>

#include "axiom_netdev.h"

/*! \brief AXIOM arm device driver data */
struct axiomnet_armdata {
    struct axiomnet_drvdata drvdata;    /*!< \brief AXIOM device driver data */
    axiom_dev_t *dev_api;               /*!< \brief AXIOM dev HW API*/

    struct device *dev;                 /*!< \brief parent device */

    /* I/O registers */
    void __iomem *vregs;                /*!< \brief Memory mapped IO registers:
                                                    virtual kernel address */
    struct resource *regs_res;          /*!< \brief IO resource */

    /* IRQ */
    int irq;                            /*!< \brief IRQ descriptor */
};


/* Match table for of_platform binding */
static const struct of_device_id axiomnet_arm_match[] = {
    { .compatible = AXIOMREG_COMPATIBLE, },
    {},
};

static irqreturn_t axiomnet_arm_irq(int irq, void *dev_id)
{
    struct axiomnet_armdata *armdata = dev_id;
    irqreturn_t serviced = IRQ_NONE;

    axiomnet_irqhandler(&armdata->drvdata);

    serviced = IRQ_HANDLED;

    return serviced;
}

static int axiomnet_arm_probe(struct platform_device *pdev)
{
    struct axiomnet_armdata *armdata;
    axiom_dev_regs_t ax_args;
    int err = 0;

    DPRINTF("start");

    /* allocate our structure and fill it out */
    armdata = kzalloc(sizeof(*armdata), GFP_KERNEL);
    if (armdata == NULL)
        return -ENOMEM;

    armdata->dev = &pdev->dev;
    platform_set_drvdata(pdev, armdata);

    /* map device registers */
    armdata->regs_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    armdata->vregs = devm_ioremap_resource(&pdev->dev, armdata->regs_res);
    if (IS_ERR(armdata->vregs)) {
        dev_err(&pdev->dev, "could not map Axiom Network regs.\n");
        err = PTR_ERR(armdata->vregs);
        goto free_local;
    }

    ax_args.vregs = armdata->vregs;

    /* allocate axiom api */
    armdata->dev_api = axiom_hw_dev_alloc(&ax_args);
    if (armdata->dev_api == NULL) {
        dev_err(&pdev->dev, "could not alloc axiom API\n");
        err = -ENOMEM;
        goto free_local;
    }

    axiom_hw_disable_irq(armdata->dev_api);

    /* setup IRQ */
    armdata->irq = platform_get_irq(pdev, 0);
    err = request_irq(armdata->irq, axiomnet_arm_irq, IRQF_SHARED,
            pdev->name, armdata);
    if (err) {
        dev_err(&pdev->dev, "could not get irq(%d) - %d\n", armdata->irq, err);
        err = -EIO;
        goto free_hw_dev;
    }

    /* probe AXIOM common driver */
    err = axiomnet_probe(&armdata->drvdata, armdata->dev_api);
    if (err) {
        goto free_irq;
    }

    IPRINTF(1, "AXIOM NIC driver loaded");
    return 0;

free_irq:
    free_irq(armdata->irq, armdata);
free_hw_dev:
    axiom_hw_dev_free(armdata->dev_api);
free_local:
    kfree(armdata);
    DPRINTF("error: %d", err);
    return err;
}

static int axiomnet_arm_remove(struct platform_device *pdev)
{
    struct axiomnet_armdata *armdata = platform_get_drvdata(pdev);

    /* remove AXIOM common driver */
    axiomnet_remove(&armdata->drvdata);

    free_irq(armdata->irq, armdata);
    axiom_hw_dev_free(armdata->dev_api);
    kfree(armdata);

    IPRINTF(1, "AXIOM NIC driver unloaded");

    return 0;
}

static struct platform_driver axiomnet_driver = {
    .probe = axiomnet_arm_probe,
    .remove = axiomnet_arm_remove,
    .driver = {
        .name = "axiom_net",
        .of_match_table = axiomnet_arm_match,
    },
};

/********************** AxiomNet Module [un]init *****************************/

/*
 * Entry point for loading the module
 *
 * Returns 0 on success, negative on failure
 */
static int __init axiomnet_arm_init(void)
{
    int err;

    /* init the AXIOM module */
    err = axiomnet_init();
    if (err) {
        goto err;
    }

    /* register driver */
    err = platform_driver_register(&axiomnet_driver);
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
static void __exit axiomnet_arm_cleanup(void)
{
    /* unregister driver */
    platform_driver_unregister(&axiomnet_driver);

    /* clean the axiom module */
    axiomnet_cleanup();
}

module_init(axiomnet_arm_init);
module_exit(axiomnet_arm_cleanup);
