#ifndef _SIPA_H_
#define _SIPA_H_

#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/if_ether.h>

enum sipa_evt_type {
	SIPA_RECEIVE,
	SIPA_ENTER_FLOWCTRL,
	SIPA_LEAVE_FLOWCTRL,
	SIPA_ERROR,
};

typedef void (*sipa_notify_cb)(void *priv, enum sipa_evt_type evt,
			       unsigned int data);

enum sipa_term_type {
	SIPA_TERM_PCIE0 = 0x10,
	SIPA_TERM_PCIE1 = 0x11,
	SIPA_TERM_PCIE2 = 0x12,
	SIPA_TERM_CP0 = 0x4,
	SIPA_TERM_CP1 = 0x5,
	SIPA_TERM_VCP = 0x6,

	SIPA_TERM_MAX = 0x20, /* max 5-bit register */
};

enum sipa_nic_id {
	SIPA_NIC_BB0,
	SIPA_NIC_BB1,
	SIPA_NIC_BB2,
	SIPA_NIC_BB3,
	SIPA_NIC_BB4,
	SIPA_NIC_BB5,
	SIPA_NIC_BB6,
	SIPA_NIC_BB7,
	SIPA_NIC_BB8,
	SIPA_NIC_BB9,
	SIPA_NIC_BB10,
	SIPA_NIC_BB11,
	SIPA_NIC_MAX,
};

struct sk_buff *sipa_recv_skb(int *netid, int index);
bool sipa_check_recv_tx_fifo_empty(void);
int sipa_nic_open(enum sipa_term_type src, int netid,
		  sipa_notify_cb cb, void *priv);
void sipa_nic_close(enum sipa_nic_id nic_id);
int sipa_nic_tx(enum sipa_nic_id nic_id, enum sipa_term_type dst,
		int netid, struct sk_buff *skb);
int sipa_nic_rx(int *netid, struct sk_buff **out_skb, int index);
int sipa_nic_rx_has_data(enum sipa_nic_id nic_id);
int sipa_nic_trigger_flow_ctrl_work(enum sipa_nic_id nic_id, int err);

u32 sipa_nic_get_filled_num(void);
void sipa_nic_restore_irq(void);
void sipa_nic_set_tx_fifo_rp(u32 rptr);
#endif
