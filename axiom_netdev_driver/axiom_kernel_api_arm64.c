/*!
 * \file axiom_kernel_api.c
 *
 * \version     v0.13
 * \date        2016-05-03
 *
 * This file contains the Axiom NIC hardware API implementation.
 *
 * Copyright (C) 2016, Evidence Srl
 * Terms of use are as specified in COPYING
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#include "axiom_nic_regs.h"
#include "axiom_nic_packets.h"

#include "axiom_kernel_api.h"

extern int verbose;

/*! \brief AXIOM HW device status */
typedef struct axiom_dev {
    axiom_dev_regs_t regs; /*!< \brief Memory mapped IO registers */
    axiom_msg_id_t next_raw_id;
} axiom_dev_t;


axiom_dev_t *
axiom_hw_dev_alloc(axiom_dev_regs_t *regs)
{
    axiom_dev_t *dev;

    dev = vmalloc(sizeof(*dev));
    dev->regs = *regs;
    dev->next_raw_id = 0;

    return dev;
}

void
axiom_hw_dev_free(axiom_dev_t *dev)
{
    vfree(dev);
}

void
axiom_hw_enable_irq(axiom_dev_t *dev)
{
    axi_gpio_write32(&dev->regs.axi.mskirq, AXIOMREG_IRQ_ALL);
}

void
axiom_hw_disable_irq(axiom_dev_t *dev)
{
    axi_gpio_write32(&dev->regs.axi.mskirq, ~(uint32_t)(AXIOMREG_IRQ_ALL));
}

uint32_t
axiom_hw_pending_irq(axiom_dev_t *dev)
{
    return axi_gpio_read32(&dev->regs.axi.pndirq);
}

void
axiom_hw_ack_irq(axiom_dev_t *dev, uint32_t ack_irq)
{
    axi_gpio_write32(&dev->regs.axi.pndirq, ack_irq);
}

axiom_err_t
axiom_hw_check_version(axiom_dev_t *dev)
{
    uint32_t version;

    /* TODO: check version */
    version = axi_gpio_read32(&dev->regs.axi.version);
    IPRINTF(verbose, "version: 0x%08x", version);

    return AXIOM_RET_OK;
}

axiom_msg_id_t
axiom_hw_raw_tx(axiom_dev_t *dev, axiom_raw_hdr_t *header,
        axiom_raw_payload_t *payload)
{
    int i, payload_size;

    header->tx.port_type.field.s = 0;
    header->tx.msg_id = dev->next_raw_id++;
    payload_size = header->tx.payload_size;

    /* write header + 4 byte of payload */
    axi_fifo_write64(&dev->regs.axi.fifo_raw_tx,
            ((uint64_t) header->raw32 << 32) |
            *((uint32_t *)&(payload->raw[0])));

    /* write payload (first 4 byte written with the header) */
    for (i = 4; i < payload_size && i < sizeof(axiom_raw_payload_t); i += 8) {
        axi_fifo_write64(&dev->regs.axi.fifo_raw_tx,
                *((uint64_t *)&(payload->raw[i])));
    }

    DPRINTF("header: %x", header->raw32);

    return header->tx.msg_id;
}

axiom_queue_len_t
axiom_hw_raw_tx_avail(axiom_dev_t *dev)
{
    return axi_fifo_tx_vacancy(&dev->regs.axi.fifo_raw_tx);
}

axiom_msg_id_t
axiom_hw_raw_rx(axiom_dev_t *dev, axiom_raw_hdr_t *header,
        axiom_raw_payload_t *payload)
{
    int i, payload_size;
    uint64_t raw;

    /* read header */
    raw = axi_fifo_read64(&dev->regs.axi.fifo_raw_rx);
    header->raw32 = ((uint32_t *)&(raw))[0];
    *((uint32_t *)&(payload->raw[0])) = ((uint32_t *)&(raw))[1];

    /* read payload */
    payload_size = header->rx.payload_size;
    for (i = 4; i < payload_size && i < sizeof(axiom_raw_payload_t); i += 8) {
        *((uint64_t *)&(payload->raw[i])) =
            axi_fifo_read64(&dev->regs.axi.fifo_raw_rx);
    }

    return header->rx.msg_id;
}

axiom_queue_len_t
axiom_hw_raw_rx_avail(axiom_dev_t *dev)
{
    return axi_fifo_rx_occupancy(&dev->regs.axi.fifo_raw_rx);
}

axiom_msg_id_t
axiom_hw_rdma_tx(axiom_dev_t *dev, axiom_rdma_hdr_t *header)
{
    uint64_t *raw = ((uint64_t *)header);

    header->tx.port_type.field.s = 0;

    axi_fifo_write64(&dev->regs.axi.fifo_rdma_tx, raw[0]);
    axi_fifo_write64(&dev->regs.axi.fifo_rdma_tx, raw[1]);

    return header->tx.msg_id;
}

axiom_queue_len_t
axiom_hw_rdma_tx_avail(axiom_dev_t *dev)
{
    return axi_fifo_tx_vacancy(&dev->regs.axi.fifo_rdma_tx);
}

axiom_msg_id_t
axiom_hw_rdma_rx(axiom_dev_t *dev, axiom_rdma_hdr_t *header)
{
    uint64_t *raw = ((uint64_t *)header);

    raw[0] = axi_fifo_read64(&dev->regs.axi.fifo_rdma_rx);
    raw[1] = axi_fifo_read64(&dev->regs.axi.fifo_rdma_rx);

    return header->rx.msg_id;
}

axiom_queue_len_t
axiom_hw_rdma_rx_avail(axiom_dev_t *dev)
{
    return axi_fifo_rx_occupancy(&dev->regs.axi.fifo_rdma_rx);
}

uint32_t
axiom_hw_read_ni_status(axiom_dev_t *dev)
{
    uint32_t ret;

    //TODO: ret = ioread32(dev->regs.vregs + AXIOMREG_IO_STATUS);

    return ret;
}

void
axiom_hw_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    //TODO: iowrite32(reg_mask, dev->regs.vregs + AXIOMREG_IO_CONTROL);
}

uint32_t
axiom_hw_read_ni_control(axiom_dev_t *dev)
{
    uint32_t ret;

    //TODO: ret = ioread32(dev->regs.vregs + AXIOMREG_IO_CONTROL);

    return ret;
}

void
axiom_hw_set_rdma_zone(axiom_dev_t *dev, uint64_t start, uint64_t end)
{
    axi_gpio_write64(&dev->regs.axi.dma_start, start);
    axi_gpio_write64(&dev->regs.axi.dma_end, end);
}

void
axiom_hw_set_long_buf(axiom_dev_t *dev, int buf_id,
        axiomreg_long_buf_t *long_buf)
{
    axi_bram_write64(&dev->regs.axi.long_buf, buf_id * AXIOMREG_SIZE_LONG_BUF,
            long_buf->raw);
}

void
axiom_hw_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    axi_gpio_write32(&dev->regs.axi.nodeid, node_id);
}

axiom_node_id_t
axiom_hw_get_node_id(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = axi_gpio_read32(&dev->regs.axi.nodeid);

    return ret;
}

axiom_err_t
axiom_hw_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    uint32_t enabled_if;

    if (!enabled_mask)
        enabled_if = AXIOMREG_ROUTING_NULL_IF;
    else
        /* Find first bit set, return as a number. */
        enabled_if = ffs(enabled_mask);

    /* the registers contains only one interface ID */
    /* TODO: set AXIOMREG_SIZE_ROUTING to 4 */
    axi_bram_write32(&dev->regs.axi.rt_phy, node_id * 4, enabled_if);
    axi_bram_write32(&dev->regs.axi.rt_rx, node_id * 4, enabled_if);
    axi_bram_write32(&dev->regs.axi.rt_tx_dma, node_id * 4, enabled_if);
    axi_bram_write32(&dev->regs.axi.rt_tx_raw, node_id * 4, enabled_if);

    return 0;
}

axiom_err_t
axiom_hw_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    uint32_t enabled_if;

    /* the register contains only one interface ID */
    /* XXX: check if it works */
    enabled_if = axi_bram_read32(&dev->regs.axi.rt_phy, node_id * 4);

    if (enabled_if == AXIOMREG_ROUTING_NULL_IF)
        *enabled_mask = 0;
    else
        *enabled_mask = (1 << enabled_if);

    return 0;
}

axiom_err_t
axiom_hw_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    *if_number = axi_gpio_read32(&dev->regs.axi.ifnumber);

    return 0;
}

axiom_err_t
axiom_hw_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    axiom_err_t err = AXIOM_RET_OK;
    uint32_t ftr = 0;

    switch(if_number) {
    case 0:
        ftr = axi_gpio_read32(&dev->regs.axi.ifinfobase0);
        break;
    case 1:
        ftr = axi_gpio_read32(&dev->regs.axi.ifinfobase1);
        break;
    default:
        err = AXIOM_RET_ERROR;
    }

    *if_features = ftr;

    return err;
}

void
axiom_print_status_reg(axiom_dev_t *dev)
{
    printk("axiom --- STATUS REGISTERS start ---\n");

    printk("axiom - version: 0x%08x\n",
            axi_gpio_read32(&dev->regs.axi.version));

    //TODO: printk("axiom - status: 0x%08x\n", axi_gpio_read32(&dev->regs.axi.status));

    printk("axiom - ifnumber: 0x%08x\n",
            axi_gpio_read32(&dev->regs.axi.ifnumber));

    printk("axiom - ifinfo[0]: 0x%02x\n",
            axi_gpio_read32(&dev->regs.axi.ifinfobase0));
    printk("axiom - ifinfo[1]: 0x%02x\n",
            axi_gpio_read32(&dev->regs.axi.ifinfobase1));
#if 0
    /* TODO */
    printk("axiom - ifinfo[2]: 0x%02x\n",
            axi_gpio_read32(&dev->regs.axi.ifinfobase2));
    printk("axiom - ifinfo[3]: 0x%02x\n",
            axi_gpio_read32(&dev->regs.axi.ifinfobase3));
#endif

    printk("axiom --- STATUS REGISTERS end ---\n");
}

void
axiom_print_control_reg(axiom_dev_t *dev)
{

    printk("axiom --- CONTROL REGISTERS start ---\n");

#if 0
    /* TODO */
    printk("axiom - control: 0x%08x\n",
            axi_gpio_read32(&dev->regs.axi.control));
#endif
    printk("axiom - nodeid: 0x%08x\n", axi_gpio_read32(&dev->regs.axi.nodeid));

    printk("axiom --- CONTROL REGISTERS end ---\n");
}

static int
axiom_printbinary(char *buf, unsigned long x, int nbits)
{
    unsigned long mask = 1UL << (nbits - 1);
    while (mask != 0) {
	*buf++ = (mask & x ? '1' : '0');
	mask >>= 1;
    }
    *buf = '\0';

    return nbits;
}

void
axiom_print_routing_reg(axiom_dev_t *dev)
{
    char bufS[64];
    int i;
    uint8_t buf8;

    printk("axiom --- ROUTING REGISTERS start ---\n");

    for (i = 0; i < 256; i++) {
        axiom_hw_get_routing(dev, i, &buf8);
        axiom_printbinary(bufS, buf8, 8);
        printk("axiom - routing[%d]: %s\n", i, bufS);
    }

    printk("axiom --- ROUTING REGISTERS end ---\n");
}

void
axiom_print_queue_reg(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk("axiom --- QUEUE REGISTERS start ---\n");

    buf32 = axi_fifo_tx_vacancy(&dev->regs.axi.fifo_raw_tx);
    printk("axiom - raw_tx_vacancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_rx_occupancy(&dev->regs.axi.fifo_raw_rx);
    printk("axiom - raw_rx_occupancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_tx_vacancy(&dev->regs.axi.fifo_rdma_tx);
    printk("axiom - rdma_tx_vacancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_rx_occupancy(&dev->regs.axi.fifo_rdma_rx);
    printk("axiom - rdma_rx_occupancy: 0x%08x\n", buf32);

    printk("axiom --- QUEUE REGISTERS end ---\n");
}
