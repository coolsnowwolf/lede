/*
 * Copyright (C) 2020 Unisoc Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <linux/ipv6.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/if_arp.h>
#include <asm/byteorder.h>
#include <linux/tty.h>
#include <linux/platform_device.h>
#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/of_device.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,4,60 ))
#include <uapi/linux/sched/types.h>
#include <linux/sched/task.h>
#endif


#include "../include/sipa.h"
#include "sipa_core.h"
#include "sipa_eth.h"

#define SIPA_RECV_BUF_LEN     1600
#define SIPA_RECV_RSVD_LEN     128

static int put_recv_array_node(struct sipa_skb_array *p,
			       struct sk_buff *skb, u64 *dma_addr)
{
	u32 pos;

	if ((p->wp - p->rp) < p->depth) {
		pos = p->wp & (p->depth - 1);
		p->array[pos].skb = skb;
		p->array[pos].dma_addr = *dma_addr;
		/*
		 * Ensure that we put the item to the fifo before
		 * we update the fifo wp.
		 */
		smp_wmb();
		p->wp++;
		return 0;
	} else {
		return -1;
	}
}

static int get_recv_array_node(struct sipa_skb_array *p,
			       struct sk_buff **skb, u64 *dma_addr)
{
	u32 pos;

	if (p->rp != p->wp) {
		pos = p->rp & (p->depth -1);
		*skb = p->array[pos].skb;
		*dma_addr = p->array[pos].dma_addr;
		/*
		 * Ensure that we remove the item from the fifo before
		 * we update the fifo rp.
		 */
		smp_wmb();
		p->rp++;
		return 0;
	} else {
		return  -1;
	}
}

static int create_recv_array(struct sipa_skb_array *p, u32 depth)
{
	p->array = kzalloc(sizeof(*p->array) * depth,
			   GFP_KERNEL);
	if (!p->array)
		return -ENOMEM;
	p->rp = 0;
	p->wp = 0;
	p->depth = depth;

	return 0;
}

static void destroy_recv_array(struct sipa_skb_array *p)
{
	kfree(p->array);

	p->array = NULL;
	p->rp = 0;
	p->wp = 0;
	p->depth = 0;
}

static struct sk_buff *alloc_recv_skb(u32 req_len, u8 rsvd)
{
	struct sk_buff *skb;
	u32 hr;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	skb = __dev_alloc_skb(req_len + rsvd, GFP_KERNEL | GFP_NOWAIT);
	if (!skb) {
		dev_err(ctrl->dev, "failed to alloc skb!\n");
		return NULL;
	}

	/* save skb ptr to skb->data */
	hr = skb_headroom(skb);
	if (hr < rsvd)
		skb_reserve(skb, rsvd - hr);

	return skb;
}

static void sipa_prepare_free_node_init(struct sipa_skb_receiver *receiver,
					u32 cnt)
{
	struct sk_buff *skb;
	u32 tmp, fail_cnt = 0;
	int i;
	u32 success_cnt = 0;
	u64 dma_addr;
	struct sipa_node_description_tag *node;
#if defined (__BIG_ENDIAN_BITFIELD)
	struct sipa_node_description_tag node_tmp;
#endif
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_cmn_fifo_cfg_tag *cmn = receiver->ep->recv_fifo;

	for (i = 0; i < cnt; i++) {
		skb = alloc_recv_skb(SIPA_RECV_BUF_LEN, receiver->rsvd);
		if (!skb) {
			fail_cnt++;
			break;
		}

		tmp = skb_headroom(skb);
		if (unlikely(tmp > SIPA_RECV_RSVD_LEN)) {
			tmp -= SIPA_RECV_RSVD_LEN;
			skb_put(skb, SIPA_RECV_BUF_LEN - tmp);
			skb_push(skb, tmp);
		} else {
			skb_put(skb, SIPA_RECV_BUF_LEN);
		}

		dma_addr = (u64)dma_map_single(ctrl->pci_dev,
					       skb->head,
					       SIPA_RECV_BUF_LEN +
					       skb_headroom(skb),
					       DMA_FROM_DEVICE);
		if (dma_mapping_error(ctrl->pci_dev, (dma_addr_t)dma_addr)) {
			dev_kfree_skb_any(skb);
			dev_err(ctrl->dev,
				"prepare free node dma map err\n");
			fail_cnt++;
			break;
		}

		node = ctrl->hal_ops.get_rx_fifo_wr(cmn->fifo_id,
						    ctrl->cmn_fifo_cfg,
						    i);
		if (!node) {
			dma_unmap_single(ctrl->pci_dev, dma_addr,
					 SIPA_RECV_BUF_LEN +
					 skb_headroom(skb),
					 DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			dev_err(ctrl->dev,
				"get node fail index = %d\n", i);
			fail_cnt++;
			break;
		}

		dma_addr += ctrl->pcie_mem_offset;
#if defined (__BIG_ENDIAN_BITFIELD)
		memset(&node_tmp, 0, sizeof(node_tmp));
		node_tmp.address = dma_addr;
		node_tmp.length = skb->len;
		node_tmp.offset = skb_headroom(skb);
		node_tmp.dst = ctrl->ep.recv_fifo->dst;
		node_tmp.src = ctrl->ep.recv_fifo->cur;
		node_tmp.intr = 0;
		node_tmp.net_id = 0;
		node_tmp.err_code = 0;
		sipa_set_node_desc((u8 *)node, (u8 *)&node_tmp);
#else
		node->address = dma_addr;
		node->length = skb->len;
		node->offset = skb_headroom(skb);
		node->dst = ctrl->ep.recv_fifo->dst;
		node->src = ctrl->ep.recv_fifo->cur;
		node->intr = 0;
		node->net_id = 0;
		node->err_code = 0;
#endif
		if (dma_addr == 0 || node->address == 0)
			pr_info("cnt = %d, i = %d, dma_addr 0x%llx, node->address 0x%llx\n",
				cnt, i, dma_addr, (long long unsigned int)node->address);		
		put_recv_array_node(&receiver->recv_array, skb, &dma_addr);
		success_cnt++;
	}
	if (fail_cnt)
		dev_err(ctrl->dev,
			"fail_cnt = %d success_cnt = %d\n",
			fail_cnt, success_cnt);
}

static void fill_free_fifo(struct sipa_skb_receiver *receiver, u32 cnt)
{
	struct sk_buff *skb;
	u32 tmp, fail_cnt = 0;
	int i;
	u32 success_cnt = 0, depth;
	u64 dma_addr;
	struct sipa_node_description_tag *node;
#if defined (__BIG_ENDIAN_BITFIELD)
	struct sipa_node_description_tag node_tmp;
#endif
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_cmn_fifo_cfg_tag *cmn = receiver->ep->recv_fifo;

	depth = cmn->rx_fifo.depth;
	if (cnt > (depth - depth / 4)) {
//		dev_warn(ctrl->dev, "free node is not enough,need fill %d\n", cnt);
		receiver->rx_danger_cnt++;
	}

	for (i = 0; i < cnt; i++) {
		skb = alloc_recv_skb(SIPA_RECV_BUF_LEN, receiver->rsvd);
		if (!skb) {
			fail_cnt++;
			break;
		}

		tmp = skb_headroom(skb);
		if (unlikely(tmp > SIPA_RECV_RSVD_LEN)) {
			tmp -= SIPA_RECV_RSVD_LEN;
			skb_put(skb, SIPA_RECV_BUF_LEN - tmp);
			skb_push(skb, tmp);
		} else {
			skb_put(skb, SIPA_RECV_BUF_LEN);
		}

		dma_addr = (u64)dma_map_single(ctrl->pci_dev,
					       skb->head,
					       SIPA_RECV_BUF_LEN +
					       skb_headroom(skb),
					       DMA_FROM_DEVICE);
		if (dma_mapping_error(ctrl->pci_dev, (dma_addr_t)dma_addr)) {
			dev_kfree_skb_any(skb);
			dev_err(ctrl->dev,
				"prepare free node dma map err\n");
			fail_cnt++;
			break;
		}
		node = ctrl->hal_ops.get_rx_fifo_wr(cmn->fifo_id,
						    ctrl->cmn_fifo_cfg,
						    i);
		if (!node) {
			dma_unmap_single(ctrl->pci_dev, dma_addr,
					 SIPA_RECV_BUF_LEN +
					 skb_headroom(skb),
					 DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			dev_err(ctrl->dev,
				"get node fail index = %d\n", i);
			fail_cnt++;
			break;
		}

		dma_addr += ctrl->pcie_mem_offset;
#if defined (__BIG_ENDIAN_BITFIELD)
		memset(&node_tmp, 0, sizeof(node_tmp));
		node_tmp.address = dma_addr;
		node_tmp.length = skb->len;
		node_tmp.offset = skb_headroom(skb);
		node_tmp.dst = ctrl->ep.recv_fifo->dst;
		node_tmp.src = ctrl->ep.recv_fifo->cur;
		node_tmp.intr = 0;
		node_tmp.net_id = 0;
		node_tmp.err_code = 0;
		sipa_set_node_desc((u8 *)node, (u8 *)&node_tmp);
#else
		node->address = dma_addr;
		node->length = skb->len;
		node->offset = skb_headroom(skb);
		node->dst = ctrl->ep.recv_fifo->dst;
		node->src = ctrl->ep.recv_fifo->cur;
		node->intr = 0;
		node->net_id = 0;
		node->err_code = 0;
#endif

		put_recv_array_node(&receiver->recv_array, skb, &dma_addr);
		success_cnt++;
	}

	if (success_cnt) {
		ctrl->hal_ops.set_rx_fifo_wr(ctrl->pci_dev,
					     cmn->fifo_id,
					     ctrl->cmn_fifo_cfg,
					     success_cnt);
		if (atomic_read(&receiver->need_fill_cnt) > 0)
			atomic_sub(success_cnt,
				   &receiver->need_fill_cnt);
	}

	if (fail_cnt)
		dev_err(ctrl->dev,
			"fill free fifo fail_cnt = %d\n", fail_cnt);
}

static void sipa_fill_free_node(struct sipa_skb_receiver *receiver, u32 cnt)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	ctrl->hal_ops.set_rx_fifo_wr(ctrl->pci_dev,
				     receiver->ep->recv_fifo->fifo_id,
				     ctrl->cmn_fifo_cfg, cnt);

	if (atomic_read(&receiver->need_fill_cnt) > 0)
		dev_info(ctrl->dev,
			 "a very serious problem, mem cover may appear\n");

	atomic_set(&receiver->need_fill_cnt, 0);
}

static void sipa_receiver_notify_cb(void *priv, enum sipa_irq_evt_type evt,
				    unsigned long data)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_skb_receiver *receiver = (struct sipa_skb_receiver *)priv;

	if (evt & SIPA_RECV_WARN_EVT) {
		dev_dbg(ctrl->dev,
			"sipa maybe poor resources evt = 0x%x\n", evt);
		receiver->tx_danger_cnt++;
	}

	sipa_dummy_recv_trigger();
}

static void sipa_free_recv_skb(struct sipa_skb_receiver *receiver)
{
	u64 addr = 0;
	struct sk_buff *recv_skb = NULL;
    while(!get_recv_array_node(&receiver->recv_array, &recv_skb, &addr))
    {
    	dev_kfree_skb_any(recv_skb);
    }
}

struct sk_buff *sipa_recv_skb(int *netid, int index)
{
	int ret = -1;
	u32 retry_cnt = 10;
	u64 addr = 0;
	struct sk_buff *recv_skb = NULL;
#if defined (__BIG_ENDIAN_BITFIELD)
	struct sipa_node_description_tag node;
#else
	struct sipa_node_description_tag *node;
#endif
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_skb_receiver *receiver = ctrl->receiver;
	enum sipa_cmn_fifo_index id = receiver->ep->recv_fifo->fifo_id;

	ret = get_recv_array_node(&receiver->recv_array,
				  &recv_skb, &addr);
read_again:
#if defined (__BIG_ENDIAN_BITFIELD)
	sipa_get_node_desc((u8 *)ctrl->hal_ops.get_tx_fifo_rp(id,
			   ctrl->cmn_fifo_cfg, index), &node);
#else
	node = ctrl->hal_ops.get_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, index);
#endif

#if defined (__BIG_ENDIAN_BITFIELD)
	if (!node.address) {
#else
	if (!node->address) {
#endif
		if (retry_cnt--) {
			udelay(1);
			goto read_again;
		}

#if defined (__BIG_ENDIAN_BITFIELD)
		dev_err(ctrl->dev, "phy addr is null = %llx\n",
			(u64)node.address);
#else
		dev_err(ctrl->dev, "phy addr is null = %llx\n",
			(u64)node->address);
#endif
		if(!ret) {
			dma_unmap_single(ctrl->pci_dev, (dma_addr_t)(addr - ctrl->pcie_mem_offset),
					 SIPA_RECV_BUF_LEN + skb_headroom(recv_skb),
					 DMA_FROM_DEVICE);
			dev_kfree_skb_any(recv_skb);
			atomic_add(1, &receiver->need_fill_cnt);
			ctrl->hal_ops.set_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, 1);
			dev_err(ctrl->dev,
				"recv addr is null, but recv_array addr:0x%llx\n",
				addr);
		}
		return NULL;
	}

	retry_cnt = 10;
check_again:
	if (ret) {
#if defined (__BIG_ENDIAN_BITFIELD)
		dev_err(ctrl->dev,
			"recv addr:0x%llx, but recv_array is empty\n",
			(u64)node.address);
#else
		dev_err(ctrl->dev,
			"recv addr:0x%llx, but recv_array is empty\n",
			(u64)node->address);
#endif
		return NULL;
#if defined (__BIG_ENDIAN_BITFIELD)
	} else if (addr != node.address && retry_cnt) {
#else
	} else if (addr != node->address && retry_cnt) {
#endif
		retry_cnt--;
		udelay(1);
#if defined (__BIG_ENDIAN_BITFIELD)
		sipa_get_node_desc((u8 *)ctrl->hal_ops.get_tx_fifo_rp(id,
				    ctrl->cmn_fifo_cfg, index), &node);
#endif
		goto check_again;
#if defined (__BIG_ENDIAN_BITFIELD)
	} else if (addr != node.address && !retry_cnt) {
#else
	} else if (addr != node->address && !retry_cnt) {
#endif
		dma_unmap_single(ctrl->pci_dev, (dma_addr_t)(addr - ctrl->pcie_mem_offset),
				 SIPA_RECV_BUF_LEN + skb_headroom(recv_skb),
				 DMA_FROM_DEVICE);
		dev_kfree_skb_any(recv_skb);
		atomic_add(1, &receiver->need_fill_cnt);
		dev_err(ctrl->dev,
			"recv addr:0x%llx, but recv_array addr:0x%llx not equal\n",
#if defined (__BIG_ENDIAN_BITFIELD)
			(u64)node.address, addr);
#else
			(u64)node->address, addr);
#endif
		ctrl->hal_ops.set_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, 1);
		return NULL;
	}
	dma_unmap_single(ctrl->pci_dev, (dma_addr_t)(addr - ctrl->pcie_mem_offset),
			 SIPA_RECV_BUF_LEN + skb_headroom(recv_skb),
			 DMA_FROM_DEVICE);

	atomic_add(1, &receiver->need_fill_cnt);
	if (atomic_read(&receiver->need_fill_cnt) > 0x30)
		wake_up(&receiver->fill_recv_waitq);

#if defined (__BIG_ENDIAN_BITFIELD)
	*netid = node.net_id;
#else
	*netid = node->net_id;
#endif
	return recv_skb;
}
EXPORT_SYMBOL(sipa_recv_skb);

static int fill_recv_thread(void *data)
{
	int ret;
	struct sipa_skb_receiver *receiver = (struct sipa_skb_receiver *)data;
	struct sched_param param = {.sched_priority = 92};
	unsigned long flags;

	sched_setscheduler(current, SCHED_RR, &param);

	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(receiver->fill_recv_waitq,
				(atomic_read(&receiver->need_fill_cnt) > 0) || receiver->run == 0);
        spin_lock_irqsave(&receiver->exit_lock, flags);
		if(receiver->run == 0) {
			spin_unlock_irqrestore(&receiver->exit_lock, flags);
        	break;
		}
		spin_unlock_irqrestore(&receiver->exit_lock, flags);
		if (!ret)
			fill_free_fifo(receiver, atomic_read(&receiver->need_fill_cnt));
	}

    sipa_free_recv_skb(receiver);
    if (receiver->recv_array.array)
    	destroy_recv_array(&receiver->recv_array);

    kfree(receiver);
	return 0;
}

bool sipa_check_recv_tx_fifo_empty(void)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = ctrl->receiver->ep->recv_fifo->fifo_id;

	if (!ctrl->remote_ready)
		return true;

	return ctrl->hal_ops.get_tx_empty_status(id, ctrl->cmn_fifo_cfg);
}
EXPORT_SYMBOL(sipa_check_recv_tx_fifo_empty);

void sipa_receiver_open_cmn_fifo(struct sipa_skb_receiver *receiver)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg = receiver->ep->recv_fifo;

	if (unlikely(!ctrl || !receiver)) {
		pr_err("ctrl %p receiver %p not ready\n", ctrl, receiver);
		return;
	}

	ctrl->hal_ops.open(fifo_cfg->fifo_id, ctrl->cmn_fifo_cfg, NULL);
	sipa_fill_free_node(receiver, fifo_cfg->rx_fifo.depth);

	ctrl->hal_ops.set_hw_intr_thres(fifo_cfg->fifo_id,
					ctrl->cmn_fifo_cfg,
					true, 64, NULL);
	/* timeout = 1 / ipa_sys_clk * 1024 * value */
	ctrl->hal_ops.set_hw_intr_timeout(fifo_cfg->fifo_id,
					  ctrl->cmn_fifo_cfg,
					  true, 0x32, NULL);

//	ctrl->hal_ops.set_intr_txfifo_full(fifo_cfg->fifo_id,
//					   ctrl->cmn_fifo_cfg,
//					   true, NULL);
}
EXPORT_SYMBOL(sipa_receiver_open_cmn_fifo);

static void sipa_receiver_init(struct sipa_skb_receiver *receiver, u32 rsvd)
{
	u32 depth;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index fifo_id = receiver->ep->recv_fifo->fifo_id;

	dev_info(ctrl->dev,
		 "fifo_id = %d rx_fifo depth = 0x%x\n",
		 receiver->ep->recv_fifo->fifo_id,
		 receiver->ep->recv_fifo->rx_fifo.depth);

	ctrl->cmn_fifo_cfg[fifo_id].irq_cb =
		(sipa_irq_notify_cb)sipa_receiver_notify_cb;
	ctrl->cmn_fifo_cfg[fifo_id].priv = receiver;

	/* reserve space for dma flushing cache issue */
	receiver->rsvd = rsvd;
	depth = receiver->ep->recv_fifo->rx_fifo.depth;

	sipa_prepare_free_node_init(receiver, depth);
}

void sipa_receiver_add_nic(struct sipa_skb_receiver *receiver,
			   struct sipa_nic *nic)
{
	int i;
	unsigned long flags;

	for (i = 0; i < receiver->nic_cnt; i++)
		if (receiver->nic_array[i] == nic)
			return;
	spin_lock_irqsave(&receiver->lock, flags);
	if (receiver->nic_cnt < SIPA_NIC_MAX)
		receiver->nic_array[receiver->nic_cnt++] = nic;
	spin_unlock_irqrestore(&receiver->lock, flags);
}
EXPORT_SYMBOL(sipa_receiver_add_nic);

void sipa_reinit_recv_array(struct sipa_skb_receiver *receiver)
{
	if (!receiver) {
		pr_err("sipa receiver is null\n");
		return;
	}

	if (!receiver->recv_array.array) {
		pr_err("sipa p->array is null\n");
		return;
	}

	receiver->recv_array.rp = 0;
	receiver->recv_array.wp = receiver->recv_array.depth;
}

int create_sipa_skb_receiver(struct sipa_endpoint *ep,
			     struct sipa_skb_receiver **receiver_pp)
{
	int ret;
	struct sipa_skb_receiver *receiver = NULL;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	receiver = kzalloc(sizeof(*receiver), GFP_KERNEL);
	if (!receiver)
		return -ENOMEM;

	receiver->ep = ep;
	receiver->rsvd = SIPA_RECV_RSVD_LEN;

	atomic_set(&receiver->need_fill_cnt, 0);

	ret = create_recv_array(&receiver->recv_array,
				receiver->ep->recv_fifo->rx_fifo.depth);
	if (ret) {
		dev_err(ctrl->dev,
			"create_sipa_sipa_receiver: recv_array kzalloc err.\n");
		kfree(receiver);
		return -ENOMEM;
	}

	spin_lock_init(&receiver->lock);
	spin_lock_init(&receiver->exit_lock);
	init_waitqueue_head(&receiver->fill_recv_waitq);

	sipa_receiver_init(receiver, SIPA_RECV_RSVD_LEN);
    receiver->run = 1;
	receiver->fill_thread = kthread_create(fill_recv_thread, receiver,
					       "sipa-fill");
	if (IS_ERR(receiver->fill_thread)) {
		dev_err(ctrl->dev, "Failed to create kthread: ipa-fill\n");
		ret = PTR_ERR(receiver->fill_thread);
		kfree(receiver->recv_array.array);
		kfree(receiver);
		return ret;
	}

	wake_up_process(receiver->fill_thread);

	*receiver_pp = receiver;
	return 0;
}
EXPORT_SYMBOL(create_sipa_skb_receiver);

void destroy_sipa_skb_receiver(struct sipa_skb_receiver *receiver)
{
        unsigned long flags;

        spin_lock_irqsave(&receiver->exit_lock, flags);
        receiver->run = 0;
        wake_up_interruptible_all(&receiver->fill_recv_waitq);
        spin_unlock_irqrestore(&receiver->exit_lock, flags);
}
EXPORT_SYMBOL(destroy_sipa_skb_receiver);
