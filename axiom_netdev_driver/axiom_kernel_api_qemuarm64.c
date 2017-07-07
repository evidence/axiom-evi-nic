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
    void __iomem *vregs;        /*!< \brief Memory mapped IO registers */
    axiom_msg_id_t next_raw_id;
} axiom_dev_t;


axiom_dev_t *
axiom_hw_dev_alloc(axiom_dev_regs_t *regs)
{
    axiom_dev_t *dev;

    dev = vmalloc(sizeof(*dev));
    dev->vregs = regs->vregs;
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
    iowrite32(AXIOMREG_IRQ_ALL, dev->vregs + AXIOMREG_IO_MSKIRQ);
}

void
axiom_hw_disable_irq(axiom_dev_t *dev)
{
    iowrite32(~(uint32_t)(AXIOMREG_IRQ_ALL), dev->vregs + AXIOMREG_IO_MSKIRQ);
}

uint32_t
axiom_hw_pending_irq(axiom_dev_t *dev)
{
    return ioread32(dev->vregs + AXIOMREG_IO_PNDIRQ);
}

void
axiom_hw_ack_irq(axiom_dev_t *dev, uint32_t ack_irq)
{
    iowrite32(ack_irq, dev->vregs + AXIOMREG_IO_PNDIRQ);
}

axiom_err_t
axiom_hw_check_version(axiom_dev_t *dev)
{
    uint32_t version;

    /* TODO: check version */
    version = ioread32(dev->vregs + AXIOMREG_IO_VERSION);
    IPRINTF(verbose, "version: 0x%08x", version);

    return AXIOM_RET_OK;
}

axiom_msg_id_t
axiom_hw_raw_tx(axiom_dev_t *dev, axiom_raw_hdr_t *header,
        axiom_raw_payload_t *payload)
{
    axiom_raw_payload_size_t payload_size;
    void __iomem *base_reg;
    int i;

    header->tx.port_type.field.s = 0;
    header->tx.msg_id = dev->next_raw_id++;
    payload_size = header->tx.payload_size;

    base_reg = dev->vregs + AXIOMREG_IO_RAW_TX_DESC;

    /* write header */
    mb();
    iowrite32(header->raw32, base_reg);
    mb();

    /* write payload */
    for (i = 0; i < payload_size && i < AXIOM_RAW_PAYLOAD_MAX_SIZE; i += 4) {
        iowrite32(*((uint32_t *)&(payload->raw[i])), base_reg);
        mb();
    }

    DPRINTF("header: %x", header->raw32);

    return header->tx.msg_id;
}

axiom_queue_len_t
axiom_hw_raw_tx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_STATUS);

    return (ret & AXIOMREG_QSTATUS_AVAIL);
}

axiom_msg_id_t
axiom_hw_raw_rx(axiom_dev_t *dev, axiom_raw_hdr_t *header,
        axiom_raw_payload_t *payload)
{
    axiom_raw_payload_size_t payload_size;
    void __iomem *base_reg;
    int i;

    base_reg = dev->vregs + AXIOMREG_IO_RAW_RX_DESC;

    /* read header */
    mb();
    header->raw32 = ioread32(base_reg);
    mb();

    /* read payload */
    payload_size = header->rx.payload_size;
    for (i = 0; i < payload_size && i < AXIOM_RAW_PAYLOAD_MAX_SIZE; i += 4) {
        *((uint32_t *)&(payload->raw[i])) = ioread32(base_reg);
        mb();
    }

    return header->rx.msg_id;
}

axiom_queue_len_t
axiom_hw_raw_rx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_STATUS);

    return (ret & AXIOMREG_QSTATUS_AVAIL);
}

axiom_msg_id_t
axiom_hw_rdma_tx(axiom_dev_t *dev, axiom_rdma_hdr_t *header)
{
    void __iomem *base_reg;
    int i;

    header->tx.port_type.field.s = 0;

    base_reg = dev->vregs + AXIOMREG_IO_RDMA_TX_DESC;

    mb();
    for (i = 0; i < AXIOM_RDMA_HEADER_SIZE - 1; i += 4) {
        iowrite32(*((uint32_t *)&(header->raw[i])), base_reg);
        mb();
    }
    /* write last byte header */
    iowrite8(header->raw[AXIOM_RDMA_HEADER_SIZE - 1], base_reg);
    mb();

    return header->tx.msg_id;
}

axiom_queue_len_t
axiom_hw_rdma_tx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RDMA_TX_STATUS);

    return (ret & AXIOMREG_QSTATUS_AVAIL);
}

axiom_msg_id_t
axiom_hw_rdma_rx(axiom_dev_t *dev, axiom_rdma_hdr_t *header)
{
    void __iomem *base_reg;
    int i;
    base_reg = dev->vregs + AXIOMREG_IO_RDMA_RX_DESC;

    mb();
    for (i = 0; i < AXIOM_RDMA_HEADER_SIZE - 1; i += 4) {
        *((uint32_t *)&(header->raw[i])) = ioread32(base_reg);
        mb();
    }
    /* read last byte header */
    header->raw[AXIOM_RDMA_HEADER_SIZE - 1] = ioread8(base_reg);
    mb();

    return header->rx.msg_id;
}

axiom_queue_len_t
axiom_hw_rdma_rx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RDMA_RX_STATUS);

    return (ret & AXIOMREG_QSTATUS_AVAIL);
}

uint32_t
axiom_hw_read_ni_status(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_STATUS);

    return ret;
}

void
axiom_hw_set_ni_control(axiom_dev_t *dev, uint32_t reg_mask)
{
    iowrite32(reg_mask, dev->vregs + AXIOMREG_IO_CONTROL);
}

uint32_t
axiom_hw_read_ni_control(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_CONTROL);

    return ret;
}

void
axiom_hw_set_rdma_zone(axiom_dev_t *dev, uint64_t start, uint64_t end)
{
    writeq(start, dev->vregs + AXIOMREG_IO_DMA_START);
    writeq(end, dev->vregs + AXIOMREG_IO_DMA_END);
}

void
axiom_hw_set_long_buf(axiom_dev_t *dev, int buf_id,
        axiomreg_long_buf_t *long_buf)
{
    writeq(long_buf->raw, dev->vregs + AXIOMREG_IO_LONG_BUF_BASE
            + (buf_id * AXIOMREG_SIZE_LONG_BUF));
}

void
axiom_hw_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    iowrite32(node_id, dev->vregs + AXIOMREG_IO_NODEID);
}

axiom_node_id_t
axiom_hw_get_node_id(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_NODEID);

    return ret;
}

axiom_err_t
axiom_hw_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    uint8_t enabled_if;

    if (!enabled_mask)
        enabled_if = AXIOMREG_ROUTING_NULL_IF;
    else
        /* Find first bit set, return as a number. */
        enabled_if = ffs(enabled_mask);

    /* the register contains only one interface ID */
    iowrite8(enabled_if, dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

    return 0;
}

axiom_err_t
axiom_hw_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    uint8_t enabled_if;

    /* the register contains only one interface ID */
    enabled_if = ioread8(dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

    if (enabled_if == AXIOMREG_ROUTING_NULL_IF)
        *enabled_mask = 0;
    else
        *enabled_mask = (1 << enabled_if);

    return 0;
}

axiom_err_t
axiom_hw_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    *if_number = ioread8(dev->vregs + AXIOMREG_IO_IFNUMBER);

    return 0;
}

axiom_err_t
axiom_hw_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    *if_features = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE + if_number);

    return 0;
}

void
axiom_print_status_reg(axiom_dev_t *dev)
{
    uint8_t buf8;
    uint32_t buf32;

    printk("axiom --- STATUS REGISTERS start ---\n");
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_VERSION);
    printk("axiom - version: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_STATUS);
    printk("axiom - status: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_IFNUMBER);
    printk("axiom - ifnumber: 0x%08x\n", buf32);

    buf8 = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE);
    printk("axiom - ifinfo[0]: 0x%02x\n", buf8);
    buf8 = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE + 1);
    printk("axiom - ifinfo[1]: 0x%02x\n", buf8);
    buf8 = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE + 2);
    printk("axiom - ifinfo[2]: 0x%02x\n", buf8);
    buf8 = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE + 3);
    printk("axiom - ifinfo[3]: 0x%02x\n", buf8);

    printk("axiom --- STATUS REGISTERS end ---\n");
}

void
axiom_print_control_reg(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk("axiom --- CONTROL REGISTERS start ---\n");
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_CONTROL);
    printk("axiom - control: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_NODEID);
    printk("axiom - nodeid: 0x%08x\n", buf32);

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
        buf8 = ioread8(dev->vregs + AXIOMREG_IO_ROUTING_BASE + i);
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

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_STATUS);
    printk("axiom - raw_tx_status: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_STATUS);
    printk("axiom - raw_rx_status: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RDMA_TX_STATUS);
    printk("axiom - rdma_tx_status: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RDMA_RX_STATUS);
    printk("axiom - rdma_rx_status: 0x%08x\n", buf32);

    printk("axiom --- QUEUE REGISTERS end ---\n");
}
