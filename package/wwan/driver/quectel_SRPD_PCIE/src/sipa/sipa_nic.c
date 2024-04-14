/* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "../include/sipa.h"
#include "sipa_core.h"

#define SIPA_CP_SRC ((1 << SIPA_TERM_CP0) | \
		(1 << SIPA_TERM_CP1) | (1 << SIPA_TERM_VCP) | \
		(1 << 0x19) | (1 << 0x18))

struct sipa_nic_statics_info {
	u32 src_mask;
	int netid;
};

static struct sipa_nic_statics_info s_spia_nic_statics[SIPA_NIC_MAX] = {
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 0,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 1,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 2,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 3,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 4,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 5,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 6,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 7,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 8,
	},
	{
		.src_mask = SIPA_CP_SRC,
		.netid = 9,
	},
};

int sipa_nic_open(enum sipa_term_type src, int netid,
		  sipa_notify_cb cb, void *priv)
{
	struct sipa_nic *nic = NULL;
	struct sk_buff *skb;
	enum sipa_nic_id nic_id = SIPA_NIC_MAX;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (!ctrl) {
		return -EINVAL;
	}
	
    if(!ctrl->remote_ready)
		return -EINVAL;

	nic_id = netid;
	dev_info(ctrl->dev, "open nic_id = %d\n", nic_id);
	if (nic_id == SIPA_NIC_MAX)
		return -EINVAL;

	if (ctrl->nic[nic_id]) {
		nic = ctrl->nic[nic_id];
		if  (atomic_read(&nic->status) == NIC_OPEN)
			return -EBUSY;
		while ((skb = skb_dequeue(&nic->rx_skb_q)) != NULL)
			dev_kfree_skb_any(skb);
	} else {
		nic = kzalloc(sizeof(*nic), GFP_KERNEL);
		if (!nic)
			return -ENOMEM;
		ctrl->nic[nic_id] = nic;
		skb_queue_head_init(&nic->rx_skb_q);
	}

	atomic_set(&nic->status, NIC_OPEN);
	nic->nic_id = nic_id;
	nic->send_ep = &ctrl->ep;
	nic->need_notify = 0;
	nic->src_mask = s_spia_nic_statics[nic_id].src_mask;
	nic->netid = netid;
	nic->cb = cb;
	nic->cb_priv = priv;
	nic->continue_notify = true;

	/* every receiver may receive cp packets */
	//sipa_receiver_add_nic(ctrl->receiver, nic);
	sipa_skb_sender_add_nic(ctrl->sender, nic);

	return nic_id;
}
EXPORT_SYMBOL(sipa_nic_open);

void sipa_nic_close(enum sipa_nic_id nic_id)
{
	struct sipa_nic *nic = NULL;
	struct sk_buff *skb;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (!ctrl) {
		dev_err(ctrl->dev, "sipa driver may not register\n");
		return;
	}

	if (nic_id == SIPA_NIC_MAX || !ctrl->nic[nic_id])
		return;

	nic = ctrl->nic[nic_id];
	nic->continue_notify = false;
	atomic_set(&nic->status, NIC_CLOSE);
	/* free all  pending skbs */
	while ((skb = skb_dequeue(&nic->rx_skb_q)) != NULL)
		dev_kfree_skb_any(skb);

	sipa_skb_sender_remove_nic(ctrl->sender, nic);
	dev_info(ctrl->dev, "close nic_id = %d\n", nic_id);
}
EXPORT_SYMBOL(sipa_nic_close);

void sipa_nic_notify_evt(struct sipa_nic *nic, enum sipa_evt_type evt)
{
	struct sipa_core *ipa = sipa_get_ctrl_pointer();
	if (!ipa->remote_ready) {
		return;
	}

	if (nic->cb)
		nic->cb(nic->cb_priv, evt, 0);
}
EXPORT_SYMBOL(sipa_nic_notify_evt);

void sipa_nic_check_flow_ctrl(void)
{
	int i;
	struct sipa_nic *nic;
	struct sipa_core *ipa = sipa_get_ctrl_pointer();

	for (i = 0; i < SIPA_NIC_MAX; i++) {
		nic = ipa->nic[i];
		if (nic && nic->rm_flow_ctrl) {
			nic->rm_flow_ctrl = false;
			nic->cb(nic->cb_priv, SIPA_LEAVE_FLOWCTRL, 0);
		}
	}
}
EXPORT_SYMBOL(sipa_nic_check_flow_ctrl);

void sipa_nic_try_notify_recv(struct sipa_nic *nic)
{
	if (atomic_read(&nic->status) == NIC_CLOSE)
		return;

	if (nic->cb)
		nic->cb(nic->cb_priv, SIPA_RECEIVE, 0);
}
EXPORT_SYMBOL(sipa_nic_try_notify_recv);

void sipa_nic_push_skb(struct sipa_nic *nic, struct sk_buff *skb)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	atomic_inc(&ctrl->recv_cnt);
	skb_queue_tail(&nic->rx_skb_q, skb);
	if (nic->rx_skb_q.qlen == 1 || nic->continue_notify)
		nic->need_notify = 1;
}
EXPORT_SYMBOL(sipa_nic_push_skb);

int sipa_nic_tx(enum sipa_nic_id nic_id, enum sipa_term_type dst,
		int netid, struct sk_buff *skb)
{
	int ret;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (!ctrl || !ctrl->sender) {
		dev_err(ctrl->dev, "sipa driver may not register\n");
		return -EINVAL;
	}

	if (!ctrl->remote_ready) {
		ctrl->nic[nic_id]->rm_flow_ctrl = true;
//		dev_err(ctrl->dev, "remote ipa not ready\n");
		return -EINPROGRESS;
	}

	ret = sipa_skb_sender_send_data(ctrl->sender, skb, dst, netid);
//	if (ret == -EAGAIN)
//		ctrl->nic[nic_id]->flow_ctrl_status = true;

	return ret;
}
EXPORT_SYMBOL(sipa_nic_tx);

int sipa_nic_rx(int *netid, struct sk_buff **out_skb, int index)
{
	struct sk_buff *skb;

	skb = sipa_recv_skb(netid, index);
	*out_skb = skb;

	return (skb) ? 0 : -ENODATA;
}
EXPORT_SYMBOL(sipa_nic_rx);

int sipa_nic_rx_has_data(enum sipa_nic_id nic_id)
{
	struct sipa_nic *nic;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (!ctrl) {
		pr_err("sipa driver may not register\n");
		return -EINVAL;
	}
	if (!ctrl->nic[nic_id] ||
	    atomic_read(&ctrl->nic[nic_id]->status) == NIC_CLOSE)
		return 0;

	nic = ctrl->nic[nic_id];

	return (!!nic->rx_skb_q.qlen);
}
EXPORT_SYMBOL(sipa_nic_rx_has_data);

int sipa_nic_trigger_flow_ctrl_work(enum sipa_nic_id nic_id, int err)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (!ctrl) {
		pr_err("sipa driver may not register\n");
		return -EINVAL;
	}

	if (!ctrl->sender)
		return -ENODEV;

	switch (err) {
	case -EAGAIN:
        //ctrl->sender->free_notify_net = true;
		schedule_work(&ctrl->flow_ctrl_work);
		break;
	default:
		dev_warn(ctrl->dev,
			 "don't have this flow ctrl err type\n");
		break;
	}

	return 0;
}
EXPORT_SYMBOL(sipa_nic_trigger_flow_ctrl_work);

u32 sipa_nic_get_filled_num(void)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
    enum sipa_cmn_fifo_index id = ctrl->receiver->ep->recv_fifo->fifo_id;

    if (!ctrl->remote_ready) {
    	dev_err(ctrl->dev, "remote sipa not ready %d\n",
    			ctrl->remote_ready);
        return 0;
    }

    return ctrl->hal_ops.recv_node_from_tx_fifo(ctrl->dev, id,
                                                   ctrl->cmn_fifo_cfg, -1);
}
EXPORT_SYMBOL(sipa_nic_get_filled_num);

void sipa_nic_restore_irq(void)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = ctrl->receiver->ep->recv_fifo->fifo_id;

	if (!ctrl->remote_ready) {
		dev_err(ctrl->dev, "remote sipa not ready %d\n",
			ctrl->remote_ready);
		return;
	}

	ctrl->hal_ops.clr_tout_th_intr(id, ctrl->cmn_fifo_cfg);
	ctrl->hal_ops.set_intr_eb(id, ctrl->cmn_fifo_cfg, true,
				  SIPA_FIFO_THRESHOLD_IRQ_EN |
				  SIPA_FIFO_DELAY_TIMER_IRQ_EN);
}
EXPORT_SYMBOL(sipa_nic_restore_irq);

void sipa_nic_set_tx_fifo_rp(u32 rptr)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = ctrl->receiver->ep->recv_fifo->fifo_id;

	if (!ctrl->remote_ready) {
		dev_err(ctrl->dev, "remote sipa not ready %d\n",
			ctrl->remote_ready);
		return;
	}

	ctrl->hal_ops.set_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, rptr);
}
EXPORT_SYMBOL(sipa_nic_set_tx_fifo_rp);
