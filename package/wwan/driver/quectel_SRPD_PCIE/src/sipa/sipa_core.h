/*
 * SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note.
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

#ifndef _SIPA_CORE_H_
#define _SIPA_CORE_H_

#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/byteorder.h>

enum sipa_cmn_fifo_index {
	SIPA_FIFO_PCIE_DL,
	SIPA_FIFO_PCIE_UL,
	SIPA_FIFO_MAX,
};

enum sipa_irq_evt_type {
	SIPA_IRQ_TX_FIFO_THRESHOLD_SW	= BIT(22),
	SIPA_IRQ_EXIT_FLOW_CTRL         = BIT(20),
	SIPA_IRQ_ENTER_FLOW_CTRL        = BIT(19),
	SIPA_IRQ_TXFIFO_FULL_INT        = BIT(18),
	SIPA_IRQ_TXFIFO_OVERFLOW        = BIT(17),
	SIPA_IRQ_ERRORCODE_IN_TX_FIFO   = BIT(16),
	SIPA_IRQ_INTR_BIT               = BIT(15),
	SIPA_IRQ_THRESHOLD              = BIT(14),
	SIPA_IRQ_DELAY_TIMER            = BIT(13),
	SIPA_IRQ_DROP_PACKT_OCCUR       = BIT(12),

	SIPA_IRQ_ERROR			= 0x0,
};

#define SIPA_FIFO_THRESHOLD_IRQ_EN		BIT(1)
#define SIPA_FIFO_DELAY_TIMER_IRQ_EN		BIT(0)

#define SIPA_PCIE_MEM_OFFSET			0x200000000ULL

enum sipa_nic_status_e {
	NIC_OPEN,
	NIC_CLOSE
};

#define SIPA_RECV_EVT (SIPA_IRQ_INTR_BIT | SIPA_IRQ_THRESHOLD | \
		       SIPA_IRQ_DELAY_TIMER | SIPA_IRQ_TX_FIFO_THRESHOLD_SW)

#define SIPA_RECV_WARN_EVT (SIPA_IRQ_TXFIFO_FULL_INT | SIPA_IRQ_TXFIFO_OVERFLOW)

#define SMSG_FLG_DELE_REQUEST		0x1
#define SMSG_FLG_DELE_RELEASE		0x2

typedef void (*sipa_irq_notify_cb)(void *priv,
				   enum sipa_irq_evt_type evt,
				   u32 data);

struct sipa_node_description_tag {
	/*soft need to set*/
	u64 address : 40;
	/*soft need to set*/
	u32 length : 20;
	/*soft need to set*/
	u16 offset : 12;
	/*soft need to set*/
	u8	net_id;
	/*soft need to set*/
	u8	src : 5;
	/*soft need to set*/
	u8	dst : 5;
	u8	prio : 3;
	u8	bear_id : 7;
	/*soft need to set*/
	u8	intr : 1;
	/*soft need to set*/
	u8	indx : 1;
	u8	err_code : 4;
	u32 reserved : 22;
} __attribute__((__packed__));

struct sipa_cmn_fifo_params {
	u32 tx_intr_delay_us;
	u32 tx_intr_threshold;
	bool flowctrl_in_tx_full;
	u32 flow_ctrl_cfg;
	u32 flow_ctrl_irq_mode;
	u32 tx_enter_flowctrl_watermark;
	u32 tx_leave_flowctrl_watermark;
	u32 rx_enter_flowctrl_watermark;
	u32 rx_leave_flowctrl_watermark;

	u32 data_ptr_cnt;
	u32 buf_size;
	dma_addr_t data_ptr;
};

struct sipa_skb_dma_addr_node {
	struct sk_buff *skb;
	u64 dma_addr;
	struct list_head list;
};

struct sipa_cmn_fifo_tag {
	u32 depth;
	u32 wr;
	u32 rd;

	u32 fifo_base_addr_l;
	u32 fifo_base_addr_h;

	void *virtual_addr;
	dma_addr_t dma_ptr;
};

struct sipa_cmn_fifo_cfg_tag {
	const char *fifo_name;

	void *priv;

	enum sipa_cmn_fifo_index fifo_id;

	bool state;
	u32 dst;
	u32 cur;

	void __iomem *fifo_reg_base;

	struct sipa_cmn_fifo_tag rx_fifo;
	struct sipa_cmn_fifo_tag tx_fifo;

	u32 enter_flow_ctrl_cnt;
	u32 exit_flow_ctrl_cnt;

	sipa_irq_notify_cb irq_cb;
};

struct sipa_endpoint {
	/* Centered on CPU/PAM */
	struct sipa_cmn_fifo_cfg_tag *send_fifo;
	struct sipa_cmn_fifo_cfg_tag *recv_fifo;

	struct sipa_cmn_fifo_params send_fifo_param;
	struct sipa_cmn_fifo_params recv_fifo_param;

	bool inited;
	bool connected;
	bool suspended;
};

struct sipa_nic {
	enum sipa_nic_id nic_id;
	struct sipa_endpoint *send_ep;
	struct sk_buff_head rx_skb_q;
	int need_notify;
	u32 src_mask;
	int netid;
	struct list_head list;
	sipa_notify_cb cb;
	void *cb_priv;
	atomic_t status;
	bool flow_ctrl_status;
	bool continue_notify;
	bool rm_flow_ctrl;
};

struct sipa_skb_array {
	struct sipa_skb_dma_addr_node *array;
	u32 rp;
	u32 wp;
	u32 depth;
};

struct sipa_skb_sender {
	struct device *dev;

	struct sipa_endpoint *ep;

	atomic_t left_cnt;
	/* To be used for add/remove nic device */
	spinlock_t nic_lock;
	/* To be used for send skb process */
	spinlock_t send_lock;
	spinlock_t exit_lock;
	struct list_head nic_list;
	struct list_head sending_list;
	struct list_head pair_free_list;
	struct sipa_skb_dma_addr_node *pair_cache;

	bool free_notify_net;
	bool ep_cover_net;
	bool send_notify_net;

	wait_queue_head_t free_waitq;

	struct task_struct *free_thread;
	struct task_struct *send_thread;

	bool init_flag;
	u32 no_mem_cnt;
	u32 no_free_cnt;
	u32 enter_flow_ctrl_cnt;
	u32 exit_flow_ctrl_cnt;
	u32 run;
};

struct sipa_skb_receiver {
	struct sipa_endpoint *ep;
	u32 rsvd;
	struct sipa_skb_array recv_array;
	wait_queue_head_t recv_waitq;
	wait_queue_head_t fill_recv_waitq;
	spinlock_t lock;
	spinlock_t exit_lock;
	u32 nic_cnt;
	atomic_t need_fill_cnt;
	struct sipa_nic *nic_array[SIPA_NIC_MAX];

	struct task_struct *fill_thread;

	u32 tx_danger_cnt;
	u32 rx_danger_cnt;      
	u32 run;
};

struct sipa_fifo_hal_ops {
	int (*open)(enum sipa_cmn_fifo_index id,
		    struct sipa_cmn_fifo_cfg_tag *cfg_base, void *cookie);
	int (*close)(enum sipa_cmn_fifo_index id,
		     struct sipa_cmn_fifo_cfg_tag *cfg_base);
	int (*set_rx_depth)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base, u32 depth);
	int (*set_tx_depth)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base, u32 depth);
	u32 (*get_rx_depth)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base);
	int (*hal_set_tx_depth)(enum sipa_cmn_fifo_index id,
				struct sipa_cmn_fifo_cfg_tag *cfg_base,
				u32 depth);
	u32 (*get_tx_depth)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base);
	int (*set_intr_drop_packet)(enum sipa_cmn_fifo_index id,
				    struct sipa_cmn_fifo_cfg_tag *cfg_base,
				    u32 enable, sipa_irq_notify_cb cb);
	int (*set_intr_error_code)(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base,
				   u32 enable, sipa_irq_notify_cb cb);
	int (*set_intr_timeout)(enum sipa_cmn_fifo_index id,
				struct sipa_cmn_fifo_cfg_tag *cfg_base,
				u32 enable, u32 time, sipa_irq_notify_cb cb);
	int (*set_hw_intr_timeout)(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base,
				   u32 enable, u32 time, sipa_irq_notify_cb cb);
	int (*set_intr_threshold)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base,
				  u32 enable, u32 cnt, sipa_irq_notify_cb cb);
	int (*set_hw_intr_thres)(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 enable, u32 cnt, sipa_irq_notify_cb cb);
	int (*set_src_dst_term)(enum sipa_cmn_fifo_index id,
				struct sipa_cmn_fifo_cfg_tag *cfg_base,
				u32 src, u32 dst);
	int (*enable_local_flowctrl_intr)(enum sipa_cmn_fifo_index id,
					  struct sipa_cmn_fifo_cfg_tag *
					  cfg_base, u32 enable, u32 irq_mode,
					  sipa_irq_notify_cb cb);
	int (*enable_remote_flowctrl_intr)(enum sipa_cmn_fifo_index id,
					   struct sipa_cmn_fifo_cfg_tag *
					   cfg_base, u32 work_mode,
					   u32 tx_entry_watermark,
					   u32 tx_exit_watermark,
					   u32 rx_entry_watermark,
					   u32 rx_exit_watermark);
	int (*set_interrupt_intr)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base,
				  u32 enable, sipa_irq_notify_cb cb);
	int (*set_intr_txfifo_overflow)(enum sipa_cmn_fifo_index id,
					struct sipa_cmn_fifo_cfg_tag *cfg_base,
					u32 enable, sipa_irq_notify_cb cb);
	int (*set_intr_txfifo_full)(enum sipa_cmn_fifo_index id,
				    struct sipa_cmn_fifo_cfg_tag *cfg_base,
				    u32 enable, sipa_irq_notify_cb cb);
	int (*put_node_to_rx_fifo)(struct device *dev,
				   enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base,
				   struct sipa_node_description_tag *node,
				   u32 force_intr, u32 num);
	u32 (*get_left_cnt)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base);
	u32 (*recv_node_from_tx_fifo)(struct device *dev,
				      enum sipa_cmn_fifo_index id,
				      struct sipa_cmn_fifo_cfg_tag *cfg_base,
				      u32 num);
	void (*get_rx_ptr)(enum sipa_cmn_fifo_index id,
			   struct sipa_cmn_fifo_cfg_tag *cfg_base,
			   u32 *wr, u32 *rd);
	void (*get_tx_ptr)(enum sipa_cmn_fifo_index id,
			   struct sipa_cmn_fifo_cfg_tag *cfg_base,
			   u32 *wr, u32 *rd);
	void (*get_filled_depth)(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 *rx_filled, u32 *tx_filled);
	u32 (*get_tx_full_status)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base);
	u32 (*get_tx_empty_status)(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base);
	u32 (*get_rx_full_status)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base);
	u32 (*get_rx_empty_status)(enum sipa_cmn_fifo_index id,
				   struct sipa_cmn_fifo_cfg_tag *cfg_base);
	bool (*set_rx_fifo_wptr)(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 wptr);
	bool (*set_tx_fifo_wptr)(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base,
				 u32 wptr);
	int (*set_rx_tx_fifo_ptr)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base,
				  u32 rx_rd, u32 rx_wr, u32 tx_rd, u32 tx_wr);
	int (*ctrl_receive)(enum sipa_cmn_fifo_index id,
			    struct sipa_cmn_fifo_cfg_tag *cfg_base,
			    bool stop);
	struct sipa_node_description_tag *
		(*get_tx_fifo_rp)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base,
				  u32 index);
	struct sipa_node_description_tag *
		(*get_rx_fifo_wr)(enum sipa_cmn_fifo_index id,
				  struct sipa_cmn_fifo_cfg_tag *cfg_base,
				  u32 index);
	int (*set_tx_fifo_rp)(enum sipa_cmn_fifo_index id,
			      struct sipa_cmn_fifo_cfg_tag *cfg_base,
			      u32 tx_rd);
	int (*set_rx_fifo_wr)(struct device *dev, enum sipa_cmn_fifo_index id,
			      struct sipa_cmn_fifo_cfg_tag *cfg_base,
			      u32 num);
	int (*set_intr_eb)(enum sipa_cmn_fifo_index id,
			   struct sipa_cmn_fifo_cfg_tag *cfg_base,
			   bool eb, u32 type);
	void (*clr_tout_th_intr)(enum sipa_cmn_fifo_index id,
				 struct sipa_cmn_fifo_cfg_tag *cfg_base);
};

struct sipa_core {
	const char *name;

	struct device *dev;
	struct device *pci_dev;
	struct dentry *dentry;
	struct sipa_endpoint ep;

	struct sipa_cmn_fifo_cfg_tag cmn_fifo_cfg[SIPA_FIFO_MAX];

	struct work_struct flow_ctrl_work;

	/* ipa low power*/
	bool remote_ready;

	struct resource *reg_res;
	phys_addr_t reg_mapped;
	void __iomem *virt_reg_addr;
	/* IPA NIC interface */
	struct sipa_nic *nic[SIPA_NIC_MAX];

	/* sender & receiver */
	struct sipa_skb_sender *sender;
	struct sipa_skb_receiver *receiver;

	atomic_t recv_cnt;
	u64 pcie_mem_offset;

	struct sipa_fifo_hal_ops hal_ops;

	struct task_struct *smsg_thread;

	struct dentry *debugfs_root;
	const void *debugfs_data;
};

void sipa_fifo_ops_init(struct sipa_fifo_hal_ops *ops);
struct sipa_core *sipa_get_ctrl_pointer(void);

void sipa_receiver_add_nic(struct sipa_skb_receiver *receiver,
			   struct sipa_nic *nic);
void sipa_receiver_open_cmn_fifo(struct sipa_skb_receiver *receiver);

void sipa_sender_open_cmn_fifo(struct sipa_skb_sender *sender);
int create_sipa_skb_sender(struct sipa_endpoint *ep,
			   struct sipa_skb_sender **sender_pp);
void destroy_sipa_skb_sender(struct sipa_skb_sender *sender);
void sipa_skb_sender_add_nic(struct sipa_skb_sender *sender,
			     struct sipa_nic *nic);
void sipa_skb_sender_remove_nic(struct sipa_skb_sender *sender,
				struct sipa_nic *nic);
int sipa_skb_sender_send_data(struct sipa_skb_sender *sender,
			      struct sk_buff *skb,
			      enum sipa_term_type dst,
			      u8 netid);
int create_sipa_skb_receiver(struct sipa_endpoint *ep,
			     struct sipa_skb_receiver **receiver_pp);

void sipa_nic_notify_evt(struct sipa_nic *nic, enum sipa_evt_type evt);
void sipa_nic_try_notify_recv(struct sipa_nic *nic);
void sipa_nic_push_skb(struct sipa_nic *nic, struct sk_buff *skb);
void sipa_nic_check_flow_ctrl(void);

int sipa_create_smsg_channel(struct sipa_core *ipa);

int sipa_init_debugfs(struct sipa_core *ipa);

int sipa_int_callback_func(int evt, void *cookie);

#if defined (__BIG_ENDIAN_BITFIELD)
static inline int sipa_get_node_desc(u8 *node_addr,
				     struct sipa_node_description_tag *node)
{
	if (!node_addr || !node)
		return -EINVAL;

	node->address = node_addr[0] + ((u32)node_addr[1] << 8) +
		((u32)node_addr[2] << 16) + ((u32)node_addr[3] << 24) +
		((u64)node_addr[4] << 32);
#if 0
	node->length = node_addr[5] + ((u32)node_addr[6] << 8) +
		((u32)(node_addr[7] & 0xf) << 16);
	node->offset = ((node_addr[7] & 0xf0) >> 4) +
		((u16)node_addr[8] << 4);
#endif
	node->net_id = node_addr[9];
	node->src = node_addr[10] & 0x1f;
#if 0
	node->dst = ((node_addr[11] & 0x3) << 3) +
		((node_addr[10] & 0xe0) >> 5);
#endif
	node->err_code = ((node_addr[12] & 0xc0) >> 6) +
		((node_addr[13] & 0x03) << 2);
#if 0
	node->prio = (node_addr[11] & 0x1c) >> 2;
	node->bear_id = ((node_addr[11] & 0xe0) >> 5) +
		((node_addr[12] & 0xf) << 3);
	node->intr = !!(node_addr[12] & BIT(4));
	node->indx = !!(node_addr[12] & BIT(5));
	node->reserved = ((node_addr[13] & 0xfc) >> 2) +
		((u32)node_addr[14] << 6) + ((u32)node_addr[15] << 14);
#endif
	smp_rmb();

	return 0;
}

static inline int sipa_set_node_desc(u8 *dst_addr, u8 *src_addr)
{
	if (!dst_addr || !src_addr)
		return -EINVAL;

	/* address */
	dst_addr[0] = src_addr[4];
	dst_addr[1] = src_addr[3];
	dst_addr[2] = src_addr[2];
	dst_addr[3] = src_addr[1];
	dst_addr[4] = src_addr[0];

	/* length */
	dst_addr[5] = (src_addr[7] >> 4) + ((src_addr[6] & 0x0f) << 4);
	dst_addr[6] = (src_addr[6] >> 4) + ((src_addr[5] & 0x0f) << 4);
	dst_addr[7] = src_addr[5] >> 4;

	/* offset */
	dst_addr[7] += ((src_addr[8] & 0x0f) << 4);
	dst_addr[8] = (src_addr[7] << 4) + (src_addr[8] >> 4);

	/* netid */
	dst_addr[9] = src_addr[9];

	/* src */
	dst_addr[10] = ((src_addr[10] & 0xf8) >> 3);

	/* dst */
	dst_addr[10] +=
		((src_addr[11] >> 6) + ((src_addr[10] & 0x01) << 2)) <<	5;
	dst_addr[11] = (src_addr[10] & 0x6) >> 1;

	/* prio */
	dst_addr[11] += ((src_addr[11] & 0x38) >> 1);

	/* bear_id */
	dst_addr[11] += ((src_addr[12] & 0x70) << 1);
	dst_addr[12] = ((src_addr[11] & 0x7) << 1) + (src_addr[12] >> 7);

	/* intx */
	dst_addr[12] += ((src_addr[12] & 0x8) << 1);

	/* indx */
	dst_addr[12] += ((src_addr[12] & 0x4) << 3);

	/* err code */
	dst_addr[12] += (src_addr[13] & 0xc0);
	dst_addr[13] = src_addr[12] & 0x3;

	/* reserved */
	dst_addr[13] += src_addr[15] << 2;
	dst_addr[14] = (src_addr[15] & 0x3) + (src_addr[14] << 2);
	dst_addr[15] = ((src_addr[13] & 0x3f) << 2) +
		((src_addr[14] & 0xc0) >> 6);
	smp_wmb();

	return 0;
}
#endif
#endif
