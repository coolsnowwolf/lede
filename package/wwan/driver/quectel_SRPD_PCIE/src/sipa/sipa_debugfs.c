#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regmap.h>
#include <linux/dma-mapping.h>
#include <uapi/linux/swab.h>

#include "../include/sipa.h"
#include "sipa_core.h"

static u32 debug_cmd[5], data_buf[5];
static struct sipa_node_description_tag ipa_node;

static int sipa_params_debug_show(struct seq_file *s, void *unused)
{
	int i;
    u32 tmp;
	struct sipa_core *ipa = (struct sipa_core *)s->private;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;

	seq_printf(s, "dma_mask = 0x%llx coherent_dma_mask = 0x%llx\n",
		   (u64)*ipa->pci_dev->dma_mask, (u64)ipa->pci_dev->coherent_dma_mask);
	seq_printf(s, "remote ready = %d reg_mapped = 0x%llx virt_reg_addr = 0x%p\n",
		   ipa->remote_ready, (long long unsigned int)ipa->reg_mapped, ipa->virt_reg_addr);
	seq_printf(s, "ipa reg start = 0x%llx size = 0x%llx pcie_mem_offset = %llx\n",
		   (long long unsigned int)ipa->reg_res->start, (long long unsigned int)resource_size(ipa->reg_res),
		   (long long unsigned int)ipa->pcie_mem_offset);
	for (i = 0; i < SIPA_NIC_MAX; i++) {
		if (!ipa->nic[i])
			continue;

		seq_printf(s, "open = %d src_mask = 0x%x netid = %d flow_ctrl_status = %d",
			   atomic_read(&ipa->nic[i]->status), ipa->nic[i]->src_mask,
			   ipa->nic[i]->netid, ipa->nic[i]->flow_ctrl_status);
		seq_printf(s, " qlen = %d need_notify = %d continue_notify = %d\n",
			   ipa->nic[i]->rx_skb_q.qlen, ipa->nic[i]->need_notify,
			   ipa->nic[i]->continue_notify);
	}

	seq_printf(s, "sender no_mem_cnt = %d no_free_cnt = %d left_cnt = %d\n",
		   ipa->sender->no_mem_cnt, ipa->sender->no_free_cnt,
		   atomic_read(&ipa->sender->left_cnt));
	seq_printf(s, "sender enter_flow_ctrl_cnt=%d, exit_flow_ctrl_cnt=%d, free_notify_net=%d, ep_cover_net=%d\n",
		   ipa->sender->enter_flow_ctrl_cnt, ipa->sender->exit_flow_ctrl_cnt,
		   ipa->sender->free_notify_net, ipa->sender->ep_cover_net);
	seq_printf(s, "receiver need_fill_cnt = %d",
		   atomic_read(&ipa->receiver->need_fill_cnt));
	seq_printf(s, " tx_danger_cnt = %d rx_danger_cnt = %d\n",
		   ipa->receiver->tx_danger_cnt, ipa->receiver->rx_danger_cnt);

	fifo_cfg = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL];
	seq_printf(s, "[PCIE_DL]state = %d fifo_reg_base = %p\n",
		   fifo_cfg->state, fifo_cfg->fifo_reg_base);
	seq_printf(s, "[PCIE_DL]rx fifo depth = 0x%x wr = 0x%x rd = 0x%x\n",
		   fifo_cfg->rx_fifo.depth,
		   fifo_cfg->rx_fifo.wr,
		   fifo_cfg->rx_fifo.rd);
	seq_printf(s, "[PCIE_DL]rx_fifo fifo_addrl = 0x%x fifo_addrh = 0x%x\n",
		   fifo_cfg->rx_fifo.fifo_base_addr_l,
		   fifo_cfg->rx_fifo.fifo_base_addr_h);
	seq_printf(s, "[PCIE_DL]rx fifo virt addr = %p\n",
		   fifo_cfg->rx_fifo.virtual_addr);
	seq_printf(s, "[PCIE_DL]tx fifo depth = 0x%x wr = 0x%x rd = 0x%x\n",
		   fifo_cfg->tx_fifo.depth, fifo_cfg->tx_fifo.wr,
		   fifo_cfg->tx_fifo.rd);
	seq_printf(s, "[PCIE_DL]tx_fifo fifo_addrl = 0x%x fifo_addrh = 0x%x\n",
		   fifo_cfg->tx_fifo.fifo_base_addr_l,
		   fifo_cfg->tx_fifo.fifo_base_addr_h);
	seq_printf(s, "[PCIE_DL]tx fifo virt addr = %p\n",
		   fifo_cfg->tx_fifo.virtual_addr);
	fifo_cfg = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL];
	seq_printf(s, "[PCIE_UL]state = %d fifo_reg_base = %p\n",
		   fifo_cfg->state, fifo_cfg->fifo_reg_base);
	seq_printf(s, "[PCIE_UL]rx fifo depth = 0x%x wr = 0x%x rd = 0x%x\n",
		   fifo_cfg->rx_fifo.depth,
		   fifo_cfg->rx_fifo.wr,
		   fifo_cfg->rx_fifo.rd);
	seq_printf(s, "[PCIE_UL]rx_fifo fifo_addrl = 0x%x fifo_addrh = 0x%x\n",
		   fifo_cfg->rx_fifo.fifo_base_addr_l,
		   fifo_cfg->rx_fifo.fifo_base_addr_h);
	seq_printf(s, "[PCIE_UL]rx fifo virt addr = %p\n",
		   fifo_cfg->rx_fifo.virtual_addr);
	seq_printf(s, "[PCIE_UL]tx fifo depth = 0x%x wr = 0x%x rd = 0x%x\n",
		   fifo_cfg->tx_fifo.depth, fifo_cfg->tx_fifo.wr,
		   fifo_cfg->tx_fifo.rd);
	seq_printf(s, "[PCIE_UL]tx_fifo fifo_addrl = 0x%x fifo_addrh = 0x%x\n",
		   fifo_cfg->tx_fifo.fifo_base_addr_l,
		   fifo_cfg->tx_fifo.fifo_base_addr_h);
	seq_printf(s, "[PCIE_UL]tx fifo virt addr = %p\n",
		   fifo_cfg->tx_fifo.virtual_addr);

	//ep: IPA_COMMON_TX_FIFO_DEPTH 0x0Cl
		tmp = readl_relaxed(ipa->virt_reg_addr + 0xc00 + 0x0C);
		seq_printf(s, "neil: read IPA_COMMON_TX_FIFO_DEPTH, value = %x\n", (tmp >> 16));
	
	//ep: IPA_COMMON_TX_FIFO_WR	0x10l
		tmp = readl_relaxed(ipa->virt_reg_addr + 0xc00 + 0x10);
		seq_printf(s, "neil: read IPA_COMMON_TX_FIFO_WR, value = %x\n", (tmp >> 16));
	
	//ep: IPA_COMMON_TX_FIFO_RD	0x14l
		tmp = readl_relaxed(ipa->virt_reg_addr + 0xc00 + 0x14);
		seq_printf(s, "neil: read IPA_COMMON_TX_FIFO_RD, value = %x\n", (tmp >> 16));
	return 0;
}

static int sipa_params_debug_open(struct inode *inode,
				  struct file *file)
{
	return single_open(file, sipa_params_debug_show,
			   inode->i_private);
}

static ssize_t sipa_endian_debug_write(struct file *f, const char __user *buf,
				    size_t size, loff_t *l)
{
	ssize_t len;
	u32 debug_cmd[24], data_buf[24];

	len = min(size, sizeof(data_buf) - 1);
	if (copy_from_user((char *)data_buf, buf, len))
		return -EFAULT;

	len = sscanf((char *)data_buf, "%x %x %x %x %x %x %x %x %x %x %x %x\n",
		     &debug_cmd[0], &debug_cmd[1], &debug_cmd[2],
		     &debug_cmd[3], &debug_cmd[4], &debug_cmd[5],
		     &debug_cmd[6], &debug_cmd[7], &debug_cmd[8],
		     &debug_cmd[9], &debug_cmd[10], &debug_cmd[11]);

	ipa_node.address = debug_cmd[0];
	ipa_node.length = debug_cmd[1];
	ipa_node.offset = debug_cmd[2];
	ipa_node.net_id = debug_cmd[3];
	ipa_node.src  = debug_cmd[4];
	ipa_node.dst  = debug_cmd[5];
	ipa_node.prio = debug_cmd[6];
	ipa_node.bear_id = debug_cmd[7];
	ipa_node.intr = debug_cmd[8];
	ipa_node.indx = debug_cmd[9];
	ipa_node.err_code = debug_cmd[10];
	ipa_node.reserved = debug_cmd[11];

	return size;
}

static int sipa_endian_debug_show(struct seq_file *s, void *unused)
{
	int i;
	u8 *byte;

	seq_printf(s, "address = 0x%llx length = 0x%x offset = 0x%x net_id = 0x%x\n",
		   (u64)ipa_node.address, ipa_node.length, ipa_node.offset,
		   ipa_node.net_id);
	seq_printf(s, "src = 0x%x dst = 0x%x prio = 0x%x bear_id = 0x%x\n",
		   ipa_node.src, ipa_node.dst, ipa_node.prio, ipa_node.bear_id);
	seq_printf(s, "intr = 0x%x indx = 0x%x err_code = 0x%x reserved = 0x%x\n",
		   ipa_node.intr, ipa_node.indx,
		   ipa_node.err_code, ipa_node.reserved);

	byte = (u8 *)&ipa_node;
	for (i = 0; i < sizeof(ipa_node); i++)
		seq_printf(s, "0x%x ", *(byte + i));

	seq_puts(s, "\n");

	return 0;
}

static const struct file_operations sipa_params_fops = {
	.open = sipa_params_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int sipa_endian_debug_open(struct inode *inode,
				  struct file *file)
{
	return single_open(file, sipa_endian_debug_show,
			   inode->i_private);
}

static const struct file_operations sipa_endian_fops = {
	.open = sipa_endian_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = sipa_endian_debug_write,
};

static ssize_t sipa_get_node_debug_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *l)
{
	int i;
	ssize_t len;
	u8 debug_cmd[16], data_buf[128];

	len = min(size, sizeof(data_buf) - 1);
	if (copy_from_user((char *)data_buf, buf, len))
		return -EFAULT;

	len = sscanf((char *)data_buf, "%4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx %4hhx\n",
		     &debug_cmd[0], &debug_cmd[1], &debug_cmd[2],
		     &debug_cmd[3], &debug_cmd[4], &debug_cmd[5],
		     &debug_cmd[6], &debug_cmd[7], &debug_cmd[8],
		     &debug_cmd[9], &debug_cmd[10], &debug_cmd[11],
		     &debug_cmd[12], &debug_cmd[13], &debug_cmd[14],
		     &debug_cmd[15]);

	for (i = 0; i < 16; i++)
		pr_err("0x%x ", debug_cmd[i]);
	pr_err("\n");

#if defined (__BIG_ENDIAN_BITFIELD)
	sipa_get_node_desc(debug_cmd, &ipa_node);
#else
	ipa_node.address = debug_cmd[4] + ((u32)debug_cmd[3] << 8) +
		((u32)debug_cmd[2] << 16) + ((u32)debug_cmd[1] << 24) +
		((u64)debug_cmd[0] << 32);
	ipa_node.net_id = debug_cmd[9];
	ipa_node.src = debug_cmd[10] & 0x1f;
	ipa_node.err_code = ((debug_cmd[13] & 0xc0) >> 6) +
		((debug_cmd[12] & 0x03) << 2);
#endif
	return size;
}

static int sipa_get_node_debug_show(struct seq_file *s, void *unused)
{
	int i;
	u8 *byte;

	seq_printf(s, "address = 0x%llx length = 0x%x offset = 0x%x net_id = 0x%x\n",
		   (u64)ipa_node.address, ipa_node.length, ipa_node.offset,
		   ipa_node.net_id);
	seq_printf(s, "src = 0x%x dst = 0x%x prio = 0x%x bear_id = 0x%x\n",
		   ipa_node.src, ipa_node.dst, ipa_node.prio, ipa_node.bear_id);
	seq_printf(s, "intr = 0x%x indx = 0x%x err_code = 0x%x reserved = 0x%x\n",
		   ipa_node.intr, ipa_node.indx,
		   ipa_node.err_code, ipa_node.reserved);

	byte = (u8 *)&ipa_node;
	for (i = 0; i < sizeof(ipa_node); i++)
		seq_printf(s, "0x%x ", *(byte + i));

	seq_puts(s, "\n");

	return 0;
}

static int sipa_get_node_debug_open(struct inode *inode,
				    struct file *file)
{
	return single_open(file, sipa_get_node_debug_show,
			   inode->i_private);
}

static const struct file_operations sipa_get_node_fops = {
	.open = sipa_get_node_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = sipa_get_node_debug_write,
};

static ssize_t sipa_set_node_debug_write(struct file *f, const char __user *buf,
					 size_t size, loff_t *l)
{
	ssize_t len;
	u32 debug_cmd[24], data_buf[24];

	len = min(size, sizeof(data_buf) - 1);
	if (copy_from_user((char *)data_buf, buf, len))
		return -EFAULT;

	len = sscanf((char *)data_buf, "%x %x %x %x %x %x %x %x %x %x %x %x\n",
		     &debug_cmd[0], &debug_cmd[1], &debug_cmd[2],
		     &debug_cmd[3], &debug_cmd[4], &debug_cmd[5],
		     &debug_cmd[6], &debug_cmd[7], &debug_cmd[8],
		     &debug_cmd[9], &debug_cmd[10], &debug_cmd[11]);

	ipa_node.address = debug_cmd[0];
	ipa_node.length = debug_cmd[1];
	ipa_node.offset = debug_cmd[2];
	ipa_node.net_id = debug_cmd[3];
	ipa_node.src  = debug_cmd[4];
	ipa_node.dst  = debug_cmd[5];
	ipa_node.prio = debug_cmd[6];
	ipa_node.bear_id = debug_cmd[7];
	ipa_node.intr = debug_cmd[8];
	ipa_node.indx = debug_cmd[9];
	ipa_node.err_code = debug_cmd[10];
	ipa_node.reserved = debug_cmd[11];

	return size;
}

static int sipa_set_node_debug_show(struct seq_file *s, void *unused)
{
#if defined (__BIG_ENDIAN_BITFIELD)
	int i;
	u8 node_buf[16];
#endif

	seq_printf(s, "address = 0x%llx length = 0x%x offset = 0x%x net_id = 0x%x\n",
		   (u64)ipa_node.address, ipa_node.length, ipa_node.offset,
		   ipa_node.net_id);
	seq_printf(s, "src = 0x%x dst = 0x%x prio = 0x%x bear_id = 0x%x\n",
		   ipa_node.src, ipa_node.dst, ipa_node.prio, ipa_node.bear_id);
	seq_printf(s, "intr = 0x%x indx = 0x%x err_code = 0x%x reserved = 0x%x\n",
		   ipa_node.intr, ipa_node.indx,
		   ipa_node.err_code, ipa_node.reserved);

#if defined (__BIG_ENDIAN_BITFIELD)
	sipa_set_node_desc(node_buf, (u8 *)&ipa_node);
	for (i = 0; i < sizeof(node_buf); i++)
		seq_printf(s, "0x%x ", node_buf[i]);
#endif

	seq_puts(s, "\n");

	return 0;
}

static int sipa_set_node_debug_open(struct inode *inode,
				    struct file *file)
{
	return single_open(file, sipa_set_node_debug_show,
			   inode->i_private);
}

static const struct file_operations sipa_set_node_fops = {
	.open = sipa_set_node_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = sipa_set_node_debug_write,
};

static ssize_t sipa_reg_debug_write(struct file *f, const char __user *buf,
				    size_t size, loff_t *l)
{
	ssize_t len;
	struct sipa_core *ipa = f->f_inode->i_private;

	len = min(size, sizeof(data_buf) - 1);
	if (copy_from_user((char *)data_buf, buf, len))
		return -EFAULT;

	len = sscanf((char *)data_buf, "%x %x %x %x %x\n",
		     &debug_cmd[0], &debug_cmd[1], &debug_cmd[2],
		     &debug_cmd[3], &debug_cmd[4]);
	if (debug_cmd[2])
		writel_relaxed(debug_cmd[1], ipa->virt_reg_addr + debug_cmd[0]);

	return size;
}

static int sipa_reg_debug_show(struct seq_file *s, void *unused)
{
	u32 tx_filled, rx_filled;
    u32 tx_wr, tx_rd, rx_wr, rx_rd; 
	struct sipa_core *ipa = (struct sipa_core *)s->private;

	seq_printf(s, "0x%x\n",
		   readl_relaxed(ipa->virt_reg_addr + debug_cmd[0]));

	seq_printf(s, "pcie dl tx fifo empty = %d full = %d rx fifo empty = %d full = %d\n",
		   ipa->hal_ops.get_tx_empty_status(SIPA_FIFO_PCIE_DL,
						     ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_tx_full_status(SIPA_FIFO_PCIE_DL,
						   ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_rx_empty_status(SIPA_FIFO_PCIE_DL,
						    ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_rx_full_status(SIPA_FIFO_PCIE_DL,
						    ipa->cmn_fifo_cfg));
	seq_printf(s, "pcie ul tx fifo empty = %d full = %d rx fifo empty = %d full = %d\n",
		   ipa->hal_ops.get_tx_empty_status(SIPA_FIFO_PCIE_UL,
						     ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_tx_full_status(SIPA_FIFO_PCIE_UL,
						   ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_rx_empty_status(SIPA_FIFO_PCIE_UL,
						    ipa->cmn_fifo_cfg),
		   ipa->hal_ops.get_rx_full_status(SIPA_FIFO_PCIE_UL,
						    ipa->cmn_fifo_cfg));
	ipa->hal_ops.get_filled_depth(SIPA_FIFO_PCIE_DL, ipa->cmn_fifo_cfg,
				      &rx_filled, &tx_filled);
	seq_printf(s, "pcie dl tx filled = 0x%x rx filled = 0x%x\n",
		   tx_filled, rx_filled);
	ipa->hal_ops.get_filled_depth(SIPA_FIFO_PCIE_UL, ipa->cmn_fifo_cfg,
				      &rx_filled, &tx_filled);
	seq_printf(s, "pcie ul tx filled = 0x%x rx filled = 0x%x\n",
		   tx_filled, rx_filled);

	ipa->hal_ops.get_rx_ptr(SIPA_FIFO_PCIE_UL, ipa->cmn_fifo_cfg, &rx_wr, &rx_rd);
	ipa->hal_ops.get_tx_ptr(SIPA_FIFO_PCIE_UL, ipa->cmn_fifo_cfg, &tx_wr, &tx_rd);
	seq_printf(s, "pcie ul rx_wr = 0x%x, rx_rd = 0x%x, tx_wr = 0x%x, tx_rd = 0x%x\n",
		   rx_wr, rx_rd, tx_wr, tx_rd);

	ipa->hal_ops.get_rx_ptr(SIPA_FIFO_PCIE_DL, ipa->cmn_fifo_cfg, &rx_wr, &rx_rd);
	ipa->hal_ops.get_tx_ptr(SIPA_FIFO_PCIE_DL, ipa->cmn_fifo_cfg, &tx_wr, &tx_rd);
	seq_printf(s, "pcie dl rx_wr = 0x%x, rx_rd = 0x%x, tx_wr = 0x%x, tx_rd = 0x%x\n",
		   rx_wr, rx_rd, tx_wr, tx_rd);
	
	sipa_int_callback_func(0, NULL);

	return 0;
}

static int sipa_reg_debug_open(struct inode *inode,
			       struct file *file)
{
	return single_open(file, sipa_reg_debug_show,
			   inode->i_private);
}

static const struct file_operations sipa_reg_debug_fops = {
	.open = sipa_reg_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = sipa_reg_debug_write,
};

static int sipa_send_test_show(struct seq_file *s, void *unused)
{
	struct sk_buff *skb = NULL;
	struct sipa_core *ipa = (struct sipa_core *)s->private;

	if (!skb) {
		skb = __dev_alloc_skb(256, GFP_KERNEL | GFP_NOWAIT);
		if (!skb) {
			dev_err(ipa->dev, "failed to alloc skb!\n");
			return 0;
		}
		skb_put(skb, 128);
		memset(skb->data, 0xE7, skb->len);

		sipa_skb_sender_send_data(ipa->sender, skb, 0x19, 0);
	}

	return 0;
}

static int sipa_send_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, sipa_send_test_show, inode->i_private);
}

static const struct file_operations sipa_send_test_fops = {
	.open = sipa_send_test_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static ssize_t sipa_nic_debug_write(struct file *f, const char __user *buf,
				    size_t size, loff_t *l)
{
	ssize_t len;
	u8 debug_cmd[24], data_buf[24];

	len = min(size, sizeof(data_buf) - 1);
	if (copy_from_user((char *)data_buf, buf, len))
		return -EFAULT;

	len = sscanf((char *)data_buf, "%4hhx %4hhx\n",
		     &debug_cmd[0], &debug_cmd[1]);
	if (debug_cmd[1])
		sipa_nic_open(debug_cmd[0], 0, NULL, NULL);
	else
		sipa_nic_close(debug_cmd[0]);

	return size;
}

static int sipa_nic_debug_show(struct seq_file *s, void *unused)
{
	//struct sk_buff *skb = NULL;
	struct sipa_core *ipa = (struct sipa_core *)s->private;
	struct sipa_cmn_fifo_cfg_tag *pcie_dl = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_DL];
	//struct sipa_cmn_fifo_cfg_tag *pcie_ul = &ipa->cmn_fifo_cfg[SIPA_FIFO_PCIE_UL];
	//struct sipa_cmn_fifo_tag *dl_tx_fifo = &pcie_dl->tx_fifo;
	struct sipa_cmn_fifo_tag *dl_rx_fifo = &pcie_dl->rx_fifo;
	//struct sipa_cmn_fifo_tag *ul_tx_fifo = &pcie_ul->tx_fifo;
	//struct sipa_cmn_fifo_tag *ul_rx_fifo = &pcie_ul->rx_fifo;
	struct sipa_node_description_tag *node;
	int i = 0;

	pr_info("dl rx_fifo addr: 0x%lx wp-%d rp-%d\n", (long unsigned int)dl_rx_fifo->virtual_addr,
		dl_rx_fifo->wr, dl_rx_fifo->rd);
	node = (struct sipa_node_description_tag *)dl_rx_fifo->virtual_addr;	
	for (i = 0; i < dl_rx_fifo->depth; i++, node++) {
		pr_info("node addr 0x%lx\n", (long unsigned int)node);
		pr_info("node info i-%d, addr-0x%llx len-%u off-%u netid-%u src-%u dst-%u pro-%u bearid-%u intr-%u indx-%u err-%u resd-%u\n",
			i, (long long unsigned int)node->address, node->length, node->offset, node->net_id,
			node->src, node->dst, node->prio, node->bear_id, node->intr,
			node->indx, node->err_code, node->reserved);
	}
	

	return 0;
}

static int sipa_nic_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, sipa_nic_debug_show, inode->i_private);
}

static const struct file_operations sipa_nic_debug_fops = {
	.open = sipa_nic_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = sipa_nic_debug_write,
};

int sipa_init_debugfs(struct sipa_core *ipa)
{
	struct dentry *root;
	struct dentry *file;

	root = debugfs_create_dir(dev_name(ipa->dev), NULL);
	if (!root) {
		dev_err(ipa->dev, "sipa create debugfs fail\n");
		return -ENOMEM;
	}

	file = debugfs_create_file("params", 0444, root, ipa,
				   &sipa_params_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create params file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("endian", 0444, root, ipa,
				   &sipa_endian_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create endian file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("get_node", 0444, root, ipa,
				   &sipa_get_node_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create endian file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("set_node", 0444, root, ipa,
				   &sipa_set_node_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create set node file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("reg", 0444, root, ipa,
				   &sipa_reg_debug_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create reg debug file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("send_test", 0444, root, ipa,
				   &sipa_send_test_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create send_test debug file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}

	file = debugfs_create_file("nic", 0444, root, ipa,
				   &sipa_nic_debug_fops);
	if (!file) {
		dev_err(ipa->dev, "sipa create nic debug file debugfs fail\n");
		debugfs_remove_recursive(root);
		return -ENOMEM;
	}
	ipa->dentry = root;
	
	return 0;
}
EXPORT_SYMBOL(sipa_init_debugfs);
