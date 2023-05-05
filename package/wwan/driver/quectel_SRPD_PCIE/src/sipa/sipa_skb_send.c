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


#include "sipa_phy_v0/sipa_fifo_phy.h"
#include "../include/sipa.h"
#include "sipa_core.h"
#include "sipa_eth.h"

#define SIPA_RECEIVER_BUF_LEN     1600

static void sipa_inform_evt_to_nics(struct sipa_skb_sender *sender,
				    enum sipa_evt_type evt)
{

	struct sipa_nic *nic;
    unsigned long flags;
	spin_lock_irqsave(&sender->nic_lock, flags);

	if(SIPA_LEAVE_FLOWCTRL == evt){
		if(sender->free_notify_net == true){
			pr_info("%s, not leave flowctl, free_notify_net is true\n", __func__);
			return;
		}
		
		if(sender->ep_cover_net == true){
			pr_info("%s, not leave flowctl, ep_cover_net is true\n", __func__);
			return;
		}

		pr_info("%s, leave flowctl\n", __func__);
		list_for_each_entry(nic, &sender->nic_list, list) {
			if (nic->flow_ctrl_status == true) {
				nic->flow_ctrl_status = false;
				sipa_nic_notify_evt(nic, evt);
			}
		}
	}else{
		pr_info("%s, enter flowctl\n", __func__);
		list_for_each_entry(nic, &sender->nic_list, list) {
			if (nic->flow_ctrl_status == false) {
				nic->flow_ctrl_status = true;
				sipa_nic_notify_evt(nic, evt);
			}
		}
	}

	spin_unlock_irqrestore(&sender->nic_lock, flags);
}

static void sipa_sender_notify_cb(void *priv, enum sipa_irq_evt_type evt,
				  unsigned long data)
{
	unsigned long flags;
	struct sipa_skb_sender *sender = (struct sipa_skb_sender *)priv;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	if (evt & SIPA_RECV_WARN_EVT) {
		dev_err(ctrl->dev,
			"sipa overflow on ep\n");
		sender->no_free_cnt++;
	}

	if (evt & SIPA_IRQ_ENTER_FLOW_CTRL) {
		spin_lock_irqsave(&sender->send_lock, flags);
        pr_info("sipa_sender_notify_cb set ep_cover_net true!!!!!\n");
		sender->enter_flow_ctrl_cnt++;
		sender->ep_cover_net = true;
		sipa_inform_evt_to_nics(sender, SIPA_ENTER_FLOWCTRL);
		spin_unlock_irqrestore(&sender->send_lock, flags);
	}

	if (evt & SIPA_IRQ_EXIT_FLOW_CTRL) {
		spin_lock_irqsave(&sender->send_lock, flags);
		sender->exit_flow_ctrl_cnt++;
		sender->ep_cover_net = false;
		sipa_inform_evt_to_nics(sender, SIPA_LEAVE_FLOWCTRL);
		spin_unlock_irqrestore(&sender->send_lock, flags);
	}
	wake_up(&sender->free_waitq);
}

static void sipa_free_sent_items(struct sipa_skb_sender *sender)
{
	bool status = false;
	unsigned long flags;
	u32 i, num, success_cnt = 0, retry_cnt = 10, failed_cnt = 0;
	struct sipa_skb_dma_addr_node *iter, *_iter;
#if defined (__BIG_ENDIAN_BITFIELD)
	struct sipa_node_description_tag node;
#else
	struct sipa_node_description_tag *node;
#endif
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = sender->ep->send_fifo->fifo_id;
	u32 tx_wr, tx_rd, rx_wr, rx_rd; 
	int exit_flow = 0;
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;
	void __iomem *fifo_base;
	u32 clr_sts = 0;
	u32 int_status = 0;
	u32 read_count = 0;

	num = ctrl->hal_ops.recv_node_from_tx_fifo(ctrl->dev, id,
						   ctrl->cmn_fifo_cfg, -1);
    for (i = 0; i < num; i++) {
		retry_cnt = 10;
#if defined (__BIG_ENDIAN_BITFIELD)
		sipa_get_node_desc((u8 *)ctrl->hal_ops.get_tx_fifo_rp(id,
				   ctrl->cmn_fifo_cfg, i), &node);
#else
		node = ctrl->hal_ops.get_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, i);
#endif

#if defined (__BIG_ENDIAN_BITFIELD)
		if (node.err_code)
			dev_err(ctrl->dev, "have node transfer err = %d\n",
				node.err_code);
#else
		if (node->err_code)
			dev_err(ctrl->dev, "have node transfer err = %d\n",
				node->err_code);
#endif

check_again:
	    spin_lock_irqsave(&sender->send_lock, flags);
		if (list_empty(&sender->sending_list)) {
			ctrl->hal_ops.get_rx_ptr(SIPA_FIFO_PCIE_UL, ctrl->cmn_fifo_cfg, &rx_wr, &rx_rd);
			ctrl->hal_ops.get_tx_ptr(SIPA_FIFO_PCIE_UL, ctrl->cmn_fifo_cfg, &tx_wr, &tx_rd);
			dev_err(ctrl->dev, "fifo id %d: send list is empty, old tx_wr=%x tx_rd=%x, rx_wr=%x, rx_rd=%x, left_cnt=%d\n",
					sender->ep->send_fifo->fifo_id, tx_wr, tx_rd, rx_wr, rx_rd, atomic_read(&sender->left_cnt));
				
            spin_unlock_irqrestore(&sender->send_lock, flags);
			goto sipa_free_end;
		}

		list_for_each_entry_safe(iter, _iter, &sender->sending_list, list) {
#if defined (__BIG_ENDIAN_BITFIELD)
			if (iter->dma_addr == node.address) {
#else
			if (iter->dma_addr == node->address) {
#endif
				list_del(&iter->list);
				list_add_tail(&iter->list,
					      &sender->pair_free_list);
				status = true;
				break;
			}
		}
        spin_unlock_irqrestore(&sender->send_lock, flags);

		if (status) {
			dma_unmap_single(ctrl->pci_dev,
					 (dma_addr_t)(iter->dma_addr - ctrl->pcie_mem_offset),
					 iter->skb->len +
					 skb_headroom(iter->skb),
					 DMA_TO_DEVICE);

			dev_kfree_skb_any(iter->skb);
			success_cnt++;
			status = false;
		} else {
			if (retry_cnt--) {
#if defined (__BIG_ENDIAN_BITFIELD)
				sipa_get_node_desc((u8 *)ctrl->hal_ops.get_tx_fifo_rp(id,
						   ctrl->cmn_fifo_cfg, i), &node);
#endif
				//dev_err(ctrl->dev, "free send skb warning, retry_cnt = %d\n", retry_cnt);
				goto check_again;
			}
			failed_cnt++;
		}
	}
	if(failed_cnt >0){
		dev_err(ctrl->dev, "can't find matching nodes num=%d\n", failed_cnt);
	}
	
	ctrl->hal_ops.set_tx_fifo_rp(id, ctrl->cmn_fifo_cfg, i);
	atomic_add(success_cnt, &sender->left_cnt);
	if (num != success_cnt)
		dev_err(ctrl->dev, "recv num = %d release num = %d\n", num, success_cnt);
	
sipa_free_end:
	if (sender->free_notify_net && atomic_read(&sender->left_cnt) > sender->ep->send_fifo->rx_fifo.depth / 4) {
		sender->free_notify_net = false;
		exit_flow = 1;
	}

	if(sender->ep_cover_net == true){
		fifo_cfg = ctrl->cmn_fifo_cfg + sender->ep->send_fifo->fifo_id;
		fifo_base = fifo_cfg->fifo_reg_base;

		int_status = ipa_phy_get_fifo_all_int_sts(fifo_base);
		
        if (int_status & IPA_INT_EXIT_FLOW_CTRL_STS) {
			exit_flow = 1;
			sender->ep_cover_net = false;
			clr_sts |= IPA_EXIT_FLOW_CONTROL_CLR_BIT;
			ipa_phy_clear_int(fifo_base, clr_sts);
			pr_info("%s, exit flow control\n", __func__);
		}else{
			pr_info("%s, still in flow control\n", __func__);
        }
	}

	if(exit_flow == 1){
	    spin_lock_irqsave(&sender->send_lock, flags);
		sipa_inform_evt_to_nics(sender, SIPA_LEAVE_FLOWCTRL);
	    spin_unlock_irqrestore(&sender->send_lock, flags);
	}
}

static bool sipa_sender_ck_unfree(struct sipa_skb_sender *sender)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = sender->ep->send_fifo->fifo_id;
    if (!ctrl->remote_ready) {
        printk("%s: need wait remote_ready!\n", __func__);
		return false;
    }

	if (ctrl->hal_ops.get_tx_empty_status(id, ctrl->cmn_fifo_cfg)) {
		ctrl->hal_ops.clr_tout_th_intr(id, ctrl->cmn_fifo_cfg);
		ctrl->hal_ops.set_intr_eb(id, ctrl->cmn_fifo_cfg, true,
					  SIPA_FIFO_THRESHOLD_IRQ_EN |
					  SIPA_FIFO_DELAY_TIMER_IRQ_EN);
		return false;
	} else {
		return true;
	}
}

static void sipa_free_send_skb(struct sipa_skb_sender *sender) 
{
	struct sipa_skb_dma_addr_node *iter, *_iter;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
    	unsigned long flags;

	spin_lock_irqsave(&sender->send_lock, flags);
	if (list_empty(&sender->sending_list)) {
		dev_err(ctrl->dev, "fifo id %d: send list is empty\n", sender->ep->send_fifo->fifo_id);
		spin_unlock_irqrestore(&sender->send_lock, flags);
		return;
	}

	list_for_each_entry_safe(iter, _iter, &sender->sending_list, list) {
		list_del(&iter->list);
		list_add_tail(&iter->list, &sender->pair_free_list);
		dma_unmap_single(ctrl->pci_dev, (dma_addr_t)(iter->dma_addr - ctrl->pcie_mem_offset),
			                iter->skb->len + skb_headroom(iter->skb), DMA_TO_DEVICE);
                                   dev_kfree_skb_any(iter->skb);
	}
	spin_unlock_irqrestore(&sender->send_lock, flags);
}

static int sipa_free_thread(void *data)
{
	struct sipa_skb_sender *sender = (struct sipa_skb_sender *)data;
	struct sched_param param = {.sched_priority = 90};
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	enum sipa_cmn_fifo_index id = sender->ep->send_fifo->fifo_id;
    unsigned long flags;
	struct sipa_nic *iter, *_iter;
	sched_setscheduler(current, SCHED_RR, &param);

	while (!kthread_should_stop()) {
		wait_event_interruptible(sender->free_waitq,
					 sender->free_notify_net || sender->run == 0 || sender->ep_cover_net || 
                     sipa_sender_ck_unfree(sender));

        spin_lock_irqsave(&sender->exit_lock, flags);                    
        if(sender->run == 0) {
			spin_unlock_irqrestore(&sender->exit_lock, flags);
			break;
        }
		spin_unlock_irqrestore(&sender->exit_lock, flags);
		sipa_free_sent_items(sender);
		
        if (!ctrl->hal_ops.get_tx_empty_status(id, ctrl->cmn_fifo_cfg)) {
			usleep_range(100, 200);
			//pr_info("%s, not empty\n", __func__);
		}
	}

    sipa_free_send_skb(sender);
	kfree(sender->pair_cache);

    list_for_each_entry_safe(iter, _iter, &sender->nic_list, list) {
        list_del(&iter->list);
        kfree(iter);
	}

	kfree(sender);

	return 0;
}

void sipa_sender_open_cmn_fifo(struct sipa_skb_sender *sender)
{
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg = sender->ep->send_fifo;

	if (unlikely(!ctrl || !sender)) {
		pr_err("ctrl %p sender %p not ready\n", ctrl, sender);
		return;
	}

	fifo_cfg->irq_cb = (sipa_irq_notify_cb)sipa_sender_notify_cb;
	fifo_cfg->priv = sender;
	ctrl->hal_ops.open(fifo_cfg->fifo_id, ctrl->cmn_fifo_cfg, NULL);

	ctrl->hal_ops.set_hw_intr_thres(fifo_cfg->fifo_id,
					ctrl->cmn_fifo_cfg, true,
					128, NULL);
	ctrl->hal_ops.set_hw_intr_timeout(fifo_cfg->fifo_id, ctrl->cmn_fifo_cfg,
					  true, 0x64, NULL);

	ctrl->hal_ops.set_intr_txfifo_full(fifo_cfg->fifo_id,
					   ctrl->cmn_fifo_cfg, true, NULL);
}
EXPORT_SYMBOL(sipa_sender_open_cmn_fifo);

int create_sipa_skb_sender(struct sipa_endpoint *ep,
			   struct sipa_skb_sender **sender_pp)
{
	int i, ret;
	struct sipa_skb_sender *sender = NULL;
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();

	dev_info(ctrl->dev, "sender create start\n");
	sender = kzalloc(sizeof(*sender), GFP_KERNEL);
	if (!sender) {
		dev_err(ctrl->dev, "alloc sender failed\n");
		return -ENOMEM;
	}

	sender->pair_cache = kcalloc(ep->send_fifo->rx_fifo.depth,
				     sizeof(struct sipa_skb_dma_addr_node),
				     GFP_KERNEL);
	if (!sender->pair_cache) {
		dev_err(ctrl->dev, "alloc sender->pair_cache fail\n");
		kfree(sender);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&sender->nic_list);
	INIT_LIST_HEAD(&sender->sending_list);
	INIT_LIST_HEAD(&sender->pair_free_list);
	spin_lock_init(&sender->nic_lock);
	spin_lock_init(&sender->send_lock);
	spin_lock_init(&sender->exit_lock);
	for (i = 0; i < ep->send_fifo->rx_fifo.depth; i++)
		list_add_tail(&((sender->pair_cache + i)->list),
			      &sender->pair_free_list);

	sender->ep = ep;

	atomic_set(&sender->left_cnt, ep->send_fifo->rx_fifo.depth / 4 * 3);

	init_waitqueue_head(&sender->free_waitq);
    sender->run = 1;
	sender->free_thread = kthread_create(sipa_free_thread, sender,
					     "sipa-free");
	if (IS_ERR(sender->free_thread)) {
		dev_err(ctrl->dev, "Failed to create kthread: ipa-free\n");
		ret = PTR_ERR(sender->free_thread);
		kfree(sender->pair_cache);
		kfree(sender);
		return ret;
	}

	*sender_pp = sender;
	wake_up_process(sender->free_thread);
	return 0;
}
EXPORT_SYMBOL(create_sipa_skb_sender);

void destroy_sipa_skb_sender(struct sipa_skb_sender *sender)
{
	unsigned long flags;

	spin_lock_irqsave(&sender->exit_lock, flags);
	sender->run = 0;
	wake_up_interruptible_all(&sender->free_waitq);
	spin_unlock_irqrestore(&sender->exit_lock, flags);
}
EXPORT_SYMBOL(destroy_sipa_skb_sender);

void sipa_skb_sender_add_nic(struct sipa_skb_sender *sender,
			     struct sipa_nic *nic)
{
	unsigned long flags;

	spin_lock_irqsave(&sender->nic_lock, flags);
	list_add_tail(&nic->list, &sender->nic_list);
	spin_unlock_irqrestore(&sender->nic_lock, flags);
}
EXPORT_SYMBOL(sipa_skb_sender_add_nic);

void sipa_skb_sender_remove_nic(struct sipa_skb_sender *sender,
				struct sipa_nic *nic)
{
	unsigned long flags;

	spin_lock_irqsave(&sender->nic_lock, flags);
	list_del(&nic->list);
	spin_unlock_irqrestore(&sender->nic_lock, flags);
}
EXPORT_SYMBOL(sipa_skb_sender_remove_nic);

int sipa_skb_sender_send_data(struct sipa_skb_sender *sender,
			      struct sk_buff *skb,
			      enum sipa_term_type dst,
			      u8 netid)
{
	unsigned long flags;
	u64 dma_addr;
	struct sipa_skb_dma_addr_node *node;
#if defined (__BIG_ENDIAN_BITFIELD)
	struct sipa_node_description_tag des;
#else
	struct sipa_node_description_tag *des;
#endif
	struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	struct sipa_cmn_fifo_cfg_tag *fifo_cfg;
	void __iomem *fifo_base;
	u32 clr_sts = 0;
	u32 int_status = 0;
	
    spin_lock_irqsave(&sender->send_lock, flags);
	
    if (sender->ep_cover_net == true){
		pr_info("%s, ep_cover_net is true, so return EAGAIN\n", __func__); 
		spin_unlock_irqrestore(&sender->send_lock, flags);
		wake_up(&sender->free_waitq);
		return -EAGAIN;
	}else{
		fifo_cfg = ctrl->cmn_fifo_cfg + sender->ep->send_fifo->fifo_id;
		fifo_base = fifo_cfg->fifo_reg_base;
		int_status = ipa_phy_get_fifo_all_int_sts(fifo_base);
	    if(int_status == 0x5FF000){
            pr_err("%s: check sts failed, maybe ep is down\n", __func__);
			spin_unlock_irqrestore(&sender->send_lock, flags);
			return -EINPROGRESS;
        }

		if (int_status & IPA_INT_ENTER_FLOW_CTRL_STS) {
            pr_info("sipa_skb_sender_send_data set ep_cover_net true!!!!!\n");
			sender->ep_cover_net = true;
			sender->enter_flow_ctrl_cnt++;
			clr_sts |= IPA_ENTRY_FLOW_CONTROL_CLR_BIT;
			ipa_phy_clear_int(fifo_base, clr_sts);
			sipa_inform_evt_to_nics(sender, SIPA_ENTER_FLOWCTRL);
			spin_unlock_irqrestore(&sender->send_lock, flags);
		    wake_up(&sender->free_waitq);
			return -EAGAIN;
		}
	}

	if (sender->free_notify_net == true){
		pr_info("%s: free_notify_net is true, so return EAGAIN\n", __func__); 
		spin_unlock_irqrestore(&sender->send_lock, flags);
		wake_up(&sender->free_waitq);
		return -EAGAIN;
	}

	if (!atomic_read(&sender->left_cnt)) {
		sender->no_free_cnt++;
		sender->free_notify_net = true;
		sipa_inform_evt_to_nics(sender, SIPA_ENTER_FLOWCTRL);
		spin_unlock_irqrestore(&sender->send_lock, flags);
		wake_up(&sender->free_waitq);
		return -EAGAIN;
	}

	dma_addr = (u64)dma_map_single(ctrl->pci_dev, skb->head,
				       skb->len + skb_headroom(skb),
				       DMA_TO_DEVICE);

	if (unlikely(dma_mapping_error(ctrl->pci_dev, (dma_addr_t)dma_addr))) {
		sender->free_notify_net = true;
		sipa_inform_evt_to_nics(sender, SIPA_ENTER_FLOWCTRL);
		spin_unlock_irqrestore(&sender->send_lock, flags);
		wake_up(&sender->free_waitq);
		return -EAGAIN;
	}

	dma_addr += ctrl->pcie_mem_offset;
#if defined (__BIG_ENDIAN_BITFIELD)
	memset(&des, 0, sizeof(des));
	des.address = dma_addr;
	des.length = skb->len;
	des.offset = skb_headroom(skb);
	des.net_id = netid;
	des.dst = dst;
	des.src = sender->ep->send_fifo->cur;
	des.err_code = 0;
	des.intr = 0;
	sipa_set_node_desc((u8 *)ctrl->hal_ops.get_rx_fifo_wr(sender->ep->send_fifo->fifo_id,
			   ctrl->cmn_fifo_cfg, 0), (u8 *)&des);
#else
	des = ctrl->hal_ops.get_rx_fifo_wr(sender->ep->send_fifo->fifo_id,
					   ctrl->cmn_fifo_cfg, 0);
	des->address = dma_addr;
	des->length = skb->len;
	des->offset = skb_headroom(skb);
	des->net_id = netid;
	des->dst = dst;
	des->src = sender->ep->send_fifo->cur;
	des->err_code = 0;
	des->intr = 0;
#endif
	node = list_first_entry(&sender->pair_free_list,
				struct sipa_skb_dma_addr_node,
				list);
	node->skb = skb;
	node->dma_addr = dma_addr;
	list_del(&node->list);
	list_add_tail(&node->list, &sender->sending_list);
	ctrl->hal_ops.set_rx_fifo_wr(ctrl->pci_dev,
				     sender->ep->send_fifo->fifo_id,
				     ctrl->cmn_fifo_cfg, 1);
	atomic_dec(&sender->left_cnt);
	spin_unlock_irqrestore(&sender->send_lock, flags);

	return 0;
}
EXPORT_SYMBOL(sipa_skb_sender_send_data);
