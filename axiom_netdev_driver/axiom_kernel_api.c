#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/wait.h>

#include "axiom_nic_regs.h"

#include "axiom_kernel_api.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence SRL");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("2016-01-22");

typedef struct axiom_dev {
    void __iomem *vregs;

    uint32_t tx_tail;
    uint32_t rx_head;
} axiom_dev_t;

axiom_dev_t *
axiom_hw_dev_alloc(void *vregs)
{
    axiom_dev_t *dev;

    dev = vmalloc(sizeof(*dev));
    dev->vregs = vregs;

    dev->tx_tail = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_TAIL);
    dev->rx_head = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_HEAD);

    return dev;
}

void
axiom_hw_dev_free(axiom_dev_t *dev)
{
    vfree(dev);
}


axiom_msg_id_t
axiom_hw_send_small(axiom_dev_t *dev, axiom_node_id_t dst_id,
        axiom_port_flag_t port_flag, axiom_payload_t *payload)
{
    uint32_t tail = dev->tx_tail;
    uint16_t header;

    header = ((dst_id << 8) | port_flag);
    /* write header */
    iowrite16(header, dev->vregs + AXIOMREG_IO_SMALL_TX_BASE + 8*(tail));
    /* write payload */
    iowrite32(*payload, dev->vregs + AXIOMREG_IO_SMALL_TX_BASE + 8*(tail) + 4);

    DPRINTF("tail: %x header: %x payload: %x", tail, header, *((uint32_t*)payload));

    dev->tx_tail++;
    /* module is expensive */
    if (dev->tx_tail == AXIOMREG_LEN_SMALL_QUEUE) {
        dev->tx_tail = 0;
    }

    return 0;
}

axiom_small_len_t
axiom_hw_small_tx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_AVAIL);

    return ret;
}

void
axiom_hw_small_tx_push(axiom_dev_t *dev, axiom_small_len_t count)
{
    /*TODO: check how many descriptors are pushed */
    iowrite32(count, dev->vregs + AXIOMREG_IO_SMALL_TX_PUSH);
    dev->tx_tail = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_TAIL);
}

axiom_msg_id_t
axiom_hw_recv_small(axiom_dev_t *dev, axiom_node_id_t *src_id,
        axiom_port_flag_t *port_flag, axiom_payload_t *payload)
{
    uint32_t head = dev->rx_head;
    uint16_t header;

    /* read header */
    header = ioread16(dev->vregs + AXIOMREG_IO_SMALL_RX_BASE + 8*(head));

    *src_id = ((header >> 8) & 0xFF);
    *port_flag = (header & 0xFF);

    /* read payload */
    *payload = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_BASE + 8*(head) + 4);

    dev->rx_head++;
    /* module is expensive */
    if (dev->rx_head == AXIOMREG_LEN_SMALL_QUEUE) {
        dev->rx_head = 0;
    }

    return 0;
}

axiom_small_len_t
axiom_hw_small_rx_avail(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_AVAIL);

    return ret;
}

void
axiom_hw_small_rx_pop(axiom_dev_t *dev, axiom_small_len_t count)
{
    /*TODO: check how many descriptors are poped */
    iowrite32(count, dev->vregs + AXIOMREG_IO_SMALL_RX_POP);
    dev->rx_head = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_HEAD);
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
axiom_print_small_queue_reg(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk("axiom --- SMALL QUEUE REGISTERS start ---\n");

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_HEAD);
    printk("axiom - small_tx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_TAIL);
    printk("axiom - small_tx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_TX_AVAIL);
    printk("axiom - small_tx_info: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_HEAD);
    printk("axiom - small_rx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_TAIL);
    printk("axiom - small_rx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_AVAIL);
    printk("axiom - small_rx_info: 0x%08x\n", buf32);

#if 0
    for (i = 55; i < 57; i++) {
        axiom_small_msg_t small_msg;
        buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_BASE + 8*i);
        printk("axiom - small_rx[%d]: %x\n", i, buf32);
        memcpy(&small_msg, &buf32, 4);
        buf32 = ioread32(dev->vregs + AXIOMREG_IO_SMALL_RX_BASE + 8*i + 4);
        printk("axiom - small_rx[%d]: %x\n", i, buf32);
        memcpy(((uint8_t*)(&small_msg) + 4) , &buf32, 4);

        printk("axiom - small_rx[%d]: src_node=%d dst_node=%d type=%d data=%x\n",
                i, small_msg.header.src_node, small_msg.header.dst_node,
                small_msg.header.type, small_msg.data.small);
        small_msg.header.src_node += 1;
        small_msg.header.dst_node -= 1;
        small_msg.header.type = 33;

        iowrite32(*((uint32_t*)(&small_msg)), dev->vregs +
                AXIOMREG_IO_SMALL_TX_BASE + 8*(i+1));
        iowrite32(*((uint32_t*)(&small_msg) + 1), dev->vregs +
                AXIOMREG_IO_SMALL_TX_BASE + 8*(i+1) + 4);
    }
    iowrite32(1, dev->vregs + AXIOMREG_IO_SMALL_RX_START);
    iowrite32(1, dev->vregs + AXIOMREG_IO_SMALL_TX_START);
#endif
    printk("axiom --- SMALL QUEUE REGISTERS end ---\n");
}
