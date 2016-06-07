/*!
 * \file axiom_kernel_api.c
 *
 * \version     v0.5
 * \date        2016-05-03
 *
 * This file contains the Axiom NIC hardware API implementation.
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence SRL");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("v0.5");

/*! \brief AXIOM HW device status */
typedef struct axiom_dev {
    void __iomem *vregs;        /*!< \brief Memory mapped IO registers */

    uint32_t tx_tail;           /*!< \brief RAW TX tail value (read on push)*/
    uint32_t rx_head;           /*!< \brief RAW RX head value (read on pop)*/
} axiom_dev_t;

axiom_dev_t *
axiom_hw_dev_alloc(void *vregs)
{
    axiom_dev_t *dev;

    dev = vmalloc(sizeof(*dev));
    dev->vregs = vregs;

    dev->tx_tail = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_TAIL);
    dev->rx_head = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_HEAD);

    return dev;
}

void
axiom_hw_dev_free(axiom_dev_t *dev)
{
    vfree(dev);
}


axiom_msg_id_t
axiom_hw_send_raw(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_type_t port_type, axiom_payload_size_t payload_size,
        axiom_payload_t *payload)
{
    uint32_t tail = dev->tx_tail;
    axiom_raw_hdr_t header;
    void __iomem *base_reg;
    int i;

    header.tx.port_type.raw = port_type;
    header.tx.dst = dst_id;
    header.tx.payload_size = payload_size;

    base_reg = dev->vregs + AXIOMREG_IO_RAW_TX_BASE +
        AXIOMREG_SIZE_RAW_QUEUE * (tail);

    /* write header */
    iowrite32(header.raw32, base_reg);

    /* write payload */
    for (i = 0; i < payload_size && i < AXIOM_RAW_PAYLOAD_MAX_SIZE; i += 4) {
        iowrite32(*((uint32_t *)&(payload->raw[i])), base_reg +
                AXIOM_RAW_HEADER_SIZE + i);
    }

    DPRINTF("tail: %x header: %x", tail, header.raw32);

    dev->tx_tail++;
    /* module is expensive */
    if (dev->tx_tail == AXIOMREG_LEN_RAW_QUEUE) {
        dev->tx_tail = 0;
    }

    return 0;
}

axiom_raw_len_t
axiom_hw_raw_tx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_AVAIL);

    return ret;
}

void
axiom_hw_raw_tx_push(axiom_dev_t *dev)
{
    iowrite32(dev->tx_tail, dev->vregs + AXIOMREG_IO_RAW_TX_TAIL);
}

axiom_msg_id_t
axiom_hw_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_type_t *port_type, axiom_payload_size_t *payload_size,
        axiom_payload_t *payload)
{
    uint32_t head = dev->rx_head;
    axiom_raw_hdr_t header;
    void __iomem *base_reg;
    int i;

    base_reg = dev->vregs + AXIOMREG_IO_RAW_RX_BASE +
        AXIOMREG_SIZE_RAW_QUEUE * (head);

    /* read header */
    header.raw32 = ioread32(base_reg);

    *src_id = header.rx.src;
    *port_type = header.rx.port_type.raw;
    *payload_size = header.rx.payload_size;

    /* read payload */
    for (i = 0; i < *payload_size && i < AXIOM_RAW_PAYLOAD_MAX_SIZE; i += 4) {
        *((uint32_t *)&(payload->raw[i])) = ioread32(base_reg +
                AXIOM_RAW_HEADER_SIZE + i);
    }

    dev->rx_head++;

    /* module is expensive */
    if (dev->rx_head == AXIOMREG_LEN_RAW_QUEUE) {
        dev->rx_head = 0;
    }

    return 0;
}

axiom_raw_len_t
axiom_hw_raw_rx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_AVAIL);

    return ret;
}

void
axiom_hw_raw_rx_pop(axiom_dev_t *dev)
{
    iowrite32(dev->rx_head, dev->vregs + AXIOMREG_IO_RAW_RX_HEAD);
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
    iowrite8(enabled_mask, dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

    return 0;
}

axiom_err_t
axiom_hw_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    *enabled_mask = ioread8(dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

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

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_HEAD);
    printk("axiom - tx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_TAIL);
    printk("axiom - tx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_AVAIL);
    printk("axiom - tx_info: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_HEAD);
    printk("axiom - rx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_TAIL);
    printk("axiom - rx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_AVAIL);
    printk("axiom - rx_info: 0x%08x\n", buf32);

    printk("axiom --- QUEUE REGISTERS end ---\n");
}
