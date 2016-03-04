#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include "axiom_nic_regs.h"

#include "axiom_kernel_api.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evidence S.R.L. - Stefano Garzarella");
MODULE_DESCRIPTION("Axiom Network Device Driver");
MODULE_VERSION("2016-01-22");

struct axiom_dev {
    void __iomem *vregs;
};

axiom_dev_t *
axiom_init_dev(void *vregs)
{
    axiom_dev_t *dev;

    dev = vmalloc(sizeof(*dev));
    dev->vregs = vregs;

    return dev;
}

void
axiom_free_dev(axiom_dev_t *dev)
{
    vfree(dev);
}

axiom_msg_id_t
axiom_send_raw(axiom_dev_t *dev, axiom_node_id_t src_node,
        axiom_node_id_t dst_node, axiom_data_t data);

axiom_msg_id_t
axiom_recv_raw(axiom_dev_t *dev, axiom_node_id_t *src_node,
        axiom_node_id_t *dst_node, axiom_data_t *data);

void
axiom_set_node_id(axiom_dev_t *dev, axiom_node_id_t node_id)
{
    iowrite32(node_id, dev->vregs + AXIOMREG_IO_NODEID);
}

axiom_node_id_t
axiom_get_node_id(axiom_dev_t *dev)
{
    uint32_t ret;

    ret = ioread32(dev->vregs + AXIOMREG_IO_NODEID);

    return ret;
}

axiom_err_t
axiom_set_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t enabled_mask)
{
    iowrite8(enabled_mask, dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

    return 0;
}

axiom_err_t
axiom_get_routing(axiom_dev_t *dev, axiom_node_id_t node_id,
        uint8_t *enabled_mask)
{
    *enabled_mask = ioread8(dev->vregs + AXIOMREG_IO_ROUTING_BASE + node_id);

    return 0;
}

axiom_err_t
axiom_get_if_number(axiom_dev_t *dev, axiom_if_id_t *if_number)
{
    *if_number = ioread8(dev->vregs + AXIOMREG_IO_IFNUMBER);

    return 0;
}

axiom_err_t
axiom_get_if_info(axiom_dev_t *dev, axiom_if_id_t if_number,
        uint8_t *if_features)
{
    *if_features = ioread8(dev->vregs + AXIOMREG_IO_IFINFO_BASE + if_number);

    return 0;
}

axiom_msg_id_t
axiom_send_raw_neighbour(axiom_dev_t *dev, uint8_t type,
        axiom_node_id_t src_node_id, axiom_node_id_t dst_node_id,
        axiom_if_id_t src_interface, axiom_if_id_t dst_interface, uint8_t data);

axiom_msg_id_t
axiom_recv_raw_neighbour (axiom_dev_t *dev, uint8_t* type,
        axiom_node_id_t* src_node_id, axiom_node_id_t* dst_node_id,
        axiom_if_id_t* src_interface, axiom_if_id_t* dst_interface,
        uint8_t* data);

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
axiom_print_raw_queue_reg(axiom_dev_t *dev)
{
    uint32_t buf32;

    printk("axiom --- RAW QUEUE REGISTERS start ---\n");

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_HEAD);
    printk("axiom - raw_tx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_TAIL);
    printk("axiom - raw_tx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_TX_INFO);
    printk("axiom - raw_tx_info: 0x%08x\n", buf32);

    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_HEAD);
    printk("axiom - raw_rx_head: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_TAIL);
    printk("axiom - raw_rx_tail: 0x%08x\n", buf32);
    buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_INFO);
    printk("axiom - raw_rx_info: 0x%08x\n", buf32);

#if 0
    for (i = 55; i < 57; i++) {
        axiom_raw_msg_t raw_msg;
        buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_BASE + 8*i);
        printk("axiom - raw_rx[%d]: %x\n", i, buf32);
        memcpy(&raw_msg, &buf32, 4);
        buf32 = ioread32(dev->vregs + AXIOMREG_IO_RAW_RX_BASE + 8*i + 4);
        printk("axiom - raw_rx[%d]: %x\n", i, buf32);
        memcpy(((uint8_t*)(&raw_msg) + 4) , &buf32, 4);

        printk("axiom - raw_rx[%d]: src_node=%d dst_node=%d type=%d data=%x\n",
                i, raw_msg.header.src_node, raw_msg.header.dst_node,
                raw_msg.header.type, raw_msg.data.raw);
        raw_msg.header.src_node += 1;
        raw_msg.header.dst_node -= 1;
        raw_msg.header.type = 33;

        iowrite32(*((uint32_t*)(&raw_msg)), dev->vregs +
                AXIOMREG_IO_RAW_TX_BASE + 8*(i+1));
        iowrite32(*((uint32_t*)(&raw_msg) + 1), dev->vregs +
                AXIOMREG_IO_RAW_TX_BASE + 8*(i+1) + 4);
    }
    iowrite32(1, dev->vregs + AXIOMREG_IO_RAW_RX_START);
    iowrite32(1, dev->vregs + AXIOMREG_IO_RAW_TX_START);
#endif
    printk("axiom --- RAW QUEUE REGISTERS end ---\n");
}
