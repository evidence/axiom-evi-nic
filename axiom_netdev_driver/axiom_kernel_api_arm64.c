/*!
 * \file axiom_kernel_api.c
 *
 * \version     v0.14
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
#include "axiom_nic_regs_arm64.h"
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
    axi_reg_write32(&dev->regs.axi.registers, AXIOMREG_IO_MSKIRQ, AXIOMREG_IRQ_ALL);
}

void
axiom_hw_disable_irq(axiom_dev_t *dev)
{
    axi_reg_write32(&dev->regs.axi.registers, AXIOMREG_IO_MSKIRQ,
            ~(uint32_t)(AXIOMREG_IRQ_ALL));
}

uint32_t
axiom_hw_pending_irq(axiom_dev_t *dev)
{
    return axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_PNDIRQ);
}

void
axiom_hw_ack_irq(axiom_dev_t *dev, uint32_t ack_irq)
{
    /* set bit to reset */
    axi_reg_write32(&dev->regs.axi.registers, AXIOMREG_IO_CLRIRQ, ack_irq);
    /* clear all bits */
    axi_reg_write32(&dev->regs.axi.registers, AXIOMREG_IO_CLRIRQ, 0x0);
}

axiom_err_t
axiom_hw_check_version(axiom_dev_t *dev)
{
    uint32_t version;

    /* TODO: check version */
    version = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_VERSION);
    IPRINTF(1, "version: 0x%08x", version);

    return AXIOM_RET_OK;
}

axiom_msg_id_t
axiom_hw_raw_tx(axiom_dev_t *dev, axiom_raw_msg_t *msg)
{
    int i, payload_size;
    uint32_t total_size = 0;

    msg->header.tx.port_type.field.s = 0;
    msg->header.tx.msg_id = dev->next_raw_id++;
    payload_size = msg->header.tx.payload_size;

    /* wait untill we have space for header and payload */
    while (axiom_hw_raw_tx_avail(dev) < sizeof(msg->header) + payload_size) {
        schedule();
    }

    /* write header + 3 byte of payload */
    axi_fifo_write64(&dev->regs.axi.fifo_raw_tx, *((uint64_t *) &msg->header));
    total_size += 8;

    /* write payload (first 3 byte written with the header) */
    for (i = 3; i < payload_size && i < sizeof(axiom_raw_payload_t); i += 8) {
        axi_fifo_write64(&dev->regs.axi.fifo_raw_tx,
                *((uint64_t *)&(msg->payload.raw[i])));
        total_size += 8;
    }

    /* write the total length */
    axi_fifo_tx_setlen(&dev->regs.axi.fifo_raw_tx, total_size);

#if 0
    IPRINTF(1, "total_size: %u - header(+3byte payload): 0x%llx",
            total_size, *((uint64_t *)&msg->header));
#endif

    return msg->header.tx.msg_id;
}

axiom_queue_len_t
axiom_hw_raw_tx_avail(axiom_dev_t *dev)
{
    return axi_fifo_tx_vacancy(&dev->regs.axi.fifo_raw_tx);
}

axiom_msg_id_t
axiom_hw_raw_rx(axiom_dev_t *dev, axiom_raw_msg_t *msg)
{
    uint32_t total_size;
    int i;

    /* *getlen() returns the size of the first packet in the FIFO */
    total_size = axi_fifo_rx_getlen(&dev->regs.axi.fifo_raw_rx);

    /* read header and 3 bytes of payload*/
    *((uint64_t *)&msg->header) = axi_fifo_read64(&dev->regs.axi.fifo_raw_rx);
    total_size -=8;

    /* read payload */
    i = 3;
    while (total_size > 0) {
        *((uint64_t *)&(msg->payload.raw[i])) =
            axi_fifo_read64(&dev->regs.axi.fifo_raw_rx);
        total_size -= 8;
        i += 8;
    }

#if 0
    IPRINTF(1, "header(+3byte payload): 0x%llx", *((uint64_t *)&msg->header));
#endif

    return msg->header.rx.msg_id;
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
    uint32_t total_size = 16;

    header->tx.port_type.field.s = 0;

    /* wait untill we have space for the RDMA descriptor */
    while (axiom_hw_rdma_tx_avail(dev) < total_size) {
        schedule();
    };

    /* write the RDMA descriptor */
    axi_fifo_write64(&dev->regs.axi.fifo_rdma_tx, raw[0]);
    axi_fifo_write64(&dev->regs.axi.fifo_rdma_tx, raw[1]);

    /* write the total length */
    axi_fifo_tx_setlen(&dev->regs.axi.fifo_rdma_tx, total_size);

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
    uint32_t total_size;

    /* *getlen() returns the size of the first packet in the FIFO */
    total_size = axi_fifo_rx_getlen(&dev->regs.axi.fifo_rdma_rx);

    /* read 2 8-bit word of the RDMA descriptor */
    raw[0] = axi_fifo_read64(&dev->regs.axi.fifo_rdma_rx);
    raw[1] = axi_fifo_read64(&dev->regs.axi.fifo_rdma_rx);

    /* XXX: debug only! To be removed */
    if (total_size != 16) {
        EPRINTF("WRONG RDMA HEADER - S: %u - size %u",
                header->rx.port_type.field.s, total_size);
        EPRINTF("occpuancy: %u - hdr[0]: 0x%llx hdr[1]: 0x%llx",
                axiom_hw_rdma_rx_avail(dev), raw[0], raw[1]);
    }

    DPRINTF("total_size: %u - hdr[0]: 0x%llx hdr[1]: 0x%llx",
            total_size, raw[0], raw[1]);

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
    uint32_t ret = 0x0;

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
    uint32_t ret = 0x0;

    //TODO: ret = ioread32(dev->regs.vregs + AXIOMREG_IO_CONTROL);

    return ret;
}

void
axiom_hw_set_rdma_zone(axiom_dev_t *dev, uint64_t start, uint64_t end)
{
    axi_reg_write64(&dev->regs.axi.registers, AXIOMREG_IO_DMA_START, start);
    axi_reg_write64(&dev->regs.axi.registers, AXIOMREG_IO_DMA_END, end);
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
    axi_reg_write32(&dev->regs.axi.registers, AXIOMREG_IO_NODEID, node_id);
}

axiom_node_id_t
axiom_hw_get_node_id(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_NODEID);

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
        enabled_if = ffs(enabled_mask) - 1;

    /*
     * the registers contains only one interface ID
     * IF 0 = loopback
     * IF 1-4 = out interfaces
     */
    /* TODO: set AXIOMREG_SIZE_ROUTING to 4 */
    axi_bram_write32(&dev->regs.axi.routing, node_id * 4, enabled_if);

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_hw_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    uint32_t enabled_if;

    /* the register contains only one interface ID */
    enabled_if = axi_bram_read32(&dev->regs.axi.routing, node_id * 4);

    if (enabled_if == AXIOMREG_ROUTING_NULL_IF)
        *enabled_mask = 0;
    else
        *enabled_mask = (1 << enabled_if);

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_hw_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    *if_number = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFNUMBER);

    return AXIOM_RET_OK;
}

axiom_err_t
axiom_hw_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    axiom_err_t err = AXIOM_RET_OK;
    uint32_t ftr = 0;

    switch(if_number) {
    case 0:
        ftr = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_1);
        break;
    case 1:
        ftr = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_2);
        break;
    case 2:
        ftr = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_3);
        break;
    case 3:
        ftr = axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_4);
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
    printk(KERN_ERR "axiom --- STATUS REGISTERS start ---\n");

    printk(KERN_ERR "axiom - version: 0x%08x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_VERSION));

#if 0
    printk(KERN_ERR "axiom - status: 0x%08x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_STATUS));
#endif

    printk(KERN_ERR "axiom - ifnumber: 0x%08x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFNUMBER));

    printk(KERN_ERR "axiom - ifinfo[0]: 0x%02x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_1));
    printk(KERN_ERR "axiom - ifinfo[1]: 0x%02x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_2));
    printk(KERN_ERR "axiom - ifinfo[2]: 0x%02x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_3));
    printk(KERN_ERR "axiom - ifinfo[3]: 0x%02x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_IFINFO_4));

    printk(KERN_ERR "axiom --- STATUS REGISTERS end ---\n");
}

void
axiom_print_control_reg(axiom_dev_t *dev)
{

    printk(KERN_ERR "axiom --- CONTROL REGISTERS start ---\n");

    printk(KERN_ERR "axiom - control: 0x%08x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_CONTROL));

    printk(KERN_ERR "axiom - nodeid: 0x%08x\n",
            axi_reg_read32(&dev->regs.axi.registers, AXIOMREG_IO_NODEID));

    printk(KERN_ERR "axiom --- CONTROL REGISTERS end ---\n");
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

    printk(KERN_ERR "axiom --- ROUTING REGISTERS start ---\n");

    for (i = 0; i < 256; i++) {
        axiom_hw_get_routing(dev, i, &buf8);
        axiom_printbinary(bufS, buf8, 8);
        printk(KERN_ERR "axiom - routing[%d]: %s\n", i, bufS);
    }

    printk(KERN_ERR "axiom --- ROUTING REGISTERS end ---\n");
}

void
axiom_print_queue_reg(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk(KERN_ERR "axiom --- QUEUE REGISTERS start ---\n");

    buf32 = axi_fifo_tx_vacancy(&dev->regs.axi.fifo_raw_tx);
    printk(KERN_ERR "axiom - raw_tx_vacancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_rx_occupancy(&dev->regs.axi.fifo_raw_rx);
    printk(KERN_ERR "axiom - raw_rx_occupancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_tx_vacancy(&dev->regs.axi.fifo_rdma_tx);
    printk(KERN_ERR "axiom - rdma_tx_vacancy: 0x%08x\n", buf32);

    buf32 = axi_fifo_rx_occupancy(&dev->regs.axi.fifo_rdma_rx);
    printk(KERN_ERR "axiom - rdma_rx_occupancy: 0x%08x\n", buf32);

    printk(KERN_ERR "axiom --- QUEUE REGISTERS end ---\n");
}

void
axiom_print_fpga_debug(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk(KERN_ERR "axiom --- FPGA DEBUG start ---\n");

    buf32 = axi_gpio_read32(&dev->regs.axi.debug);
    printk(KERN_ERR "axiom - FPGA GPIO debug: 0x%02x\n", buf32);

    printk(KERN_ERR "axiom --- FPGA DEBUG end ---\n");
    return;
}
