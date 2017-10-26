/*!
 * \file axiom_netdev_arm.c
 *
 * \version     v0.14
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
#include <linux/of_address.h>

#include "axiom_netdev.h"
#include "axiom_xilinx.h"
#include "axiom_nic_regs_arm64.h"

/*! \brief AXIOM arm device driver data */
struct axiomnet_armdata {
    struct axiomnet_drvdata drvdata;    /*!< \brief AXIOM device driver data */
    axiom_dev_t *dev_api;               /*!< \brief AXIOM dev HW API*/

    struct device *dev;                 /*!< \brief parent device */

    /* I/O registers */
    axiom_dev_regs_t regs;              /*!< \brief Memory mapped IO registers
                                                    virtual kernel address */
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

#if 0
    IPRINTF(1, "IRQ arrived!");
    IPRINTF(1, "before PNDIRQ: 0x%x", axiom_hw_pending_irq(armdata->dev_api));
    IPRINTF(1, "RAW RX occupancy %u", axi_fifo_rx_occupancy(&armdata->regs.axi.fifo_raw_rx));
    IPRINTF(1, "RDMA RX occupancy %u", axi_fifo_rx_occupancy(&armdata->regs.axi.fifo_rdma_rx));
    IPRINTF(1, "before PNDIRQ: 0x%x", axiom_hw_pending_irq(armdata->dev_api));
    IPRINTF(1, "after PNDIRQ: 0x%x", axiom_hw_pending_irq(armdata->dev_api));
#endif

    axiomnet_irqhandler(&armdata->drvdata);


    serviced = IRQ_HANDLED;

    return serviced;
}

static int axiomnet_reg_init(struct axiomnet_armdata *armdata,
        const char *reg_name, struct axi_reg *reg)
{
    struct device_node *node;
    struct resource axi_res;
    int err = 0;

    node = of_parse_phandle(armdata->dev->of_node, reg_name, 0);
    if (IS_ERR(node)) {
        dev_err(armdata->dev, "%s: not found in the device-tree\n",
                reg_name);
        err = PTR_ERR(node);
        goto error;
    }

    err = of_address_to_resource(node, 0, &axi_res);
    if (err) {
        dev_err(armdata->dev, "%s: resource 0 not found in the device-tree\n",
                reg_name);
        goto error;
    }

    reg->base_addr = devm_ioremap_resource(armdata->dev, &axi_res);
    if (IS_ERR(reg->base_addr)) {
        dev_err(armdata->dev, "%s: resource 0 could not map\n", reg_name);
        err = PTR_ERR(node);
        goto error;
    }

    DPRINTF("%s mapped - paddr: 0x%llx vaddr: %p", reg_name,
            axi_res.start, reg->base_addr);

error:
    return err;
}

static int axiomnet_fifo_init(struct axiomnet_armdata *armdata,
        const char *fifo_name, struct axi_fifo *fifo)
{
    struct device_node *node;
    struct resource axi_res;
    int err = 0;

    node = of_parse_phandle(armdata->dev->of_node, fifo_name, 0);
    if (IS_ERR(node)) {
        dev_err(armdata->dev, "%s: not found in the device-tree\n",
                fifo_name);
        err = PTR_ERR(node);
        goto error;
    }

    err = of_address_to_resource(node, 0, &axi_res);
    if (err) {
        dev_err(armdata->dev, "%s: resource 0 not found in the device-tree\n",
                fifo_name);
        goto error;
    }

    fifo->base_addr = devm_ioremap_resource(armdata->dev, &axi_res);
    if (IS_ERR(fifo->base_addr)) {
        dev_err(armdata->dev, "%s: resource 0 could not map\n", fifo_name);
        err = PTR_ERR(node);
        goto error;
    }

    DPRINTF("%s AXI mapped - paddr: 0x%llx vaddr: %p", fifo_name,
            axi_res.start, fifo->base_addr);

    err = of_address_to_resource(node, 1, &axi_res);
    if (err) {
        dev_err(armdata->dev, "%s: resource 1 not found in the device-tree\n",
                fifo_name);
        goto error;
    }

    fifo->axi4_base_addr = devm_ioremap_resource(armdata->dev, &axi_res);
    if (IS_ERR(fifo->axi4_base_addr)) {
        dev_err(armdata->dev, "%s: resource 1 could not map\n", fifo_name);
        err = PTR_ERR(node);
        goto error;
    }

    DPRINTF("%s AXI4 mapped - paddr: 0x%llx vaddr: %p", fifo_name,
            axi_res.start, fifo->axi4_base_addr);

    err = of_property_read_u32(node, "xlnx,data-interface-type", &fifo->axi4);
    if (err) {
        dev_err(armdata->dev,
                "%s: data-interface-type not found in the device-tree\n",
                fifo_name);
        goto error;
    }

    axi_fifo_reset(fifo);

    DPRINTF("TX vacancy %u", axi_fifo_tx_vacancy(fifo));
    DPRINTF("RX occupancy %u",axi_fifo_rx_occupancy(fifo));

error:
    return err;
}

static int axiomnet_bram_init(struct axiomnet_armdata *armdata,
        const char *bram_name, struct axi_bram *bram)
{
    struct device_node *node;
    struct resource axi_res;
    int err = 0;

    node = of_parse_phandle(armdata->dev->of_node, bram_name, 0);
    if (IS_ERR(node)) {
        dev_err(armdata->dev, "%s: not found in the device-tree\n",
                bram_name);
        err = PTR_ERR(node);
        goto error;
    }

    err = of_address_to_resource(node, 0, &axi_res);
    if (err) {
        dev_err(armdata->dev, "%s: resource 0 not found in the device-tree\n",
                bram_name);
        goto error;
    }

    bram->base_addr = devm_ioremap_resource(armdata->dev, &axi_res);
    if (IS_ERR(bram->base_addr)) {
        dev_err(armdata->dev, "%s: resource 0 could not map\n", bram_name);
        err = PTR_ERR(node);
        goto error;
    }

    DPRINTF("%s mapped - paddr: 0x%llx vaddr: %p", bram_name,
            axi_res.start, bram->base_addr);

error:
    return err;
}



static int axiomnet_axi_init(struct axiomnet_armdata *armdata)
{
    struct res_to_init {
        const char *name;
        void *res;
    };

#define REG_NUM         1
    struct res_to_init reg_to_init[REG_NUM] = {
        {"registers", &armdata->regs.axi.registers},
    };

#define FIFO_NUM        4
    struct res_to_init fifo_to_init[FIFO_NUM] = {
        {"fifo-raw-tx", &armdata->regs.axi.fifo_raw_tx},
        {"fifo-raw-rx", &armdata->regs.axi.fifo_raw_rx},
        {"fifo-rdma-tx", &armdata->regs.axi.fifo_rdma_tx},
        {"fifo-rdma-rx", &armdata->regs.axi.fifo_rdma_rx}
    };

#define BRAM_NUM        2
    struct res_to_init bram_to_init[BRAM_NUM] = {
        {"bram-long-buf", &armdata->regs.axi.long_buf},
        {"bram-routing", &armdata->regs.axi.routing},
    };

    int i, err = 0;

    /* init registers */
    for (i = 0; i < REG_NUM; i++) {
        err = axiomnet_reg_init(armdata, reg_to_init[i].name,
                reg_to_init[i].res);
        if (err) {
            goto error;
        }
    }

    /* init FIFO registers */
    for (i = 0; i < FIFO_NUM; i++) {
        err = axiomnet_fifo_init(armdata, fifo_to_init[i].name,
                fifo_to_init[i].res);
        if (err) {
            goto error;
        }
    }

    /* init BRAM registers */
    for (i = 0; i < BRAM_NUM; i++) {
        err = axiomnet_bram_init(armdata, bram_to_init[i].name,
                bram_to_init[i].res);
        if (err) {
            goto error;
        }
    }

    /* Reset Aurora IPs */
    axi_reg_write32(&armdata->regs.axi.registers, AXIOMREG_IO_CONTROL,
            AXIOMREG_CONTROL_RESET);
    axi_reg_write32(&armdata->regs.axi.registers, AXIOMREG_IO_CONTROL,
            0x0);

error:
    return err;
}

static int axiomnet_arm_probe(struct platform_device *pdev)
{
    struct axiomnet_armdata *armdata;
    int err = 0;

    DPRINTF("start");

    /* allocate our structure and fill it out */
    armdata = kzalloc(sizeof(*armdata), GFP_KERNEL);
    if (armdata == NULL)
        return -ENOMEM;

    armdata->dev = &pdev->dev;
    platform_set_drvdata(pdev, armdata);

    err = axiomnet_axi_init(armdata);
    if (err) {
        goto free_local;
    }

    /* allocate axiom api */
    armdata->dev_api = axiom_hw_dev_alloc(&armdata->regs);
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

    IPRINTF(1, "IRQ mapped - irq: %d", armdata->irq);

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
