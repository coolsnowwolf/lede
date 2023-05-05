/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2014-2016 Zhiqiang Yang <zhiqiang.yang@mediatek.com>
 */
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_bridge.h>
#include "hnat.h"

/* *
 * mcast_entry_get - Returns the index of an unused entry
 * or an already existed entry in mtbl
 */
static int mcast_entry_get(u16 vlan_id, u32 dst_mac)
{
	int index = -1;
	u8 i;
	struct ppe_mcast_group *p = hnat_priv->pmcast->mtbl;
	u8 max = hnat_priv->pmcast->max_entry;

	for (i = 0; i < max; i++) {
		if ((index == -1) && (!p->valid)) {
			index = i; /*get the first unused entry index*/
			continue;
		}
		if ((p->vid == vlan_id) && (p->mac_hi == dst_mac)) {
			index = i;
			break;
		}
		p++;
	}
	if (index == -1)
		pr_info("%s:group table is full\n", __func__);

	return index;
}

static void get_mac_from_mdb_entry(struct br_mdb_entry *entry,
				   u32 *mac_hi, u16 *mac_lo)
{
	switch (ntohs(entry->addr.proto)) {
	case ETH_P_IP:
		*mac_lo = 0x0100;
		*mac_hi = swab32((entry->addr.u.ip4 & 0xfffffe00) + 0x5e);
		break;
	case ETH_P_IPV6:
		*mac_lo = 0x3333;
		*mac_hi = swab32(entry->addr.u.ip6.s6_addr32[3]);
		break;
	}
	trace_printk("%s:group mac_h=0x%08x, mac_l=0x%04x\n",
		     __func__, *mac_hi, *mac_lo);
}

/*set_hnat_mtbl - set ppe multicast register*/
static int set_hnat_mtbl(struct ppe_mcast_group *group, u32 ppe_id, int index)
{
	struct ppe_mcast_h mcast_h;
	struct ppe_mcast_l mcast_l;
	u16 mac_lo = group->mac_lo;
	u32 mac_hi = group->mac_hi;
	u8 mc_port = group->mc_port;
	void __iomem *reg;

	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	mcast_h.u.value = 0;
	mcast_l.addr = 0;
	if (mac_lo == 0x0100)
		mcast_h.u.info.mc_mpre_sel = 0;
	else if (mac_lo == 0x3333)
		mcast_h.u.info.mc_mpre_sel = 1;

	mcast_h.u.info.mc_px_en = mc_port;
	mcast_l.addr = mac_hi;
	mcast_h.u.info.valid = group->valid;
	trace_printk("%s:index=%d,group info=0x%x,addr=0x%x\n",
		     __func__, index, mcast_h.u.value, mcast_l.addr);
	if (index < 0x10) {
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_H_0 + ((index) * 8);
		writel(mcast_h.u.value, reg);
		reg = hnat_priv->ppe_base[ppe_id] + PPE_MCAST_L_0 + ((index) * 8);
		writel(mcast_l.addr, reg);
	} else {
		index = index - 0x10;
		reg = hnat_priv->fe_base + PPE_MCAST_H_10 + ((index) * 8);
		writel(mcast_h.u.value, reg);
		reg = hnat_priv->fe_base + PPE_MCAST_L_10 + ((index) * 8);
		writel(mcast_h.u.value, reg);
	}

	return 0;
}

/**
 * hnat_mcast_table_update -
 *	1.get a valid group entry
 *	2.update group info
 *		a.update eif&oif count
 *		b.eif ==0 & oif == 0,delete it from group table
 *		c.oif != 0,set mc forward port to cpu,else do not forward to cpu
 *	3.set the group info to ppe register
 */
static int hnat_mcast_table_update(int type, struct br_mdb_entry *entry)
{
	struct net_device *dev;
	u32 mac_hi = 0;
	u16 mac_lo = 0;
	int i, index;
	struct ppe_mcast_group *group;

	rcu_read_lock();
	dev = dev_get_by_index_rcu(&init_net, entry->ifindex);
	if (!dev) {
		rcu_read_unlock();
		return -ENODEV;
	}
	rcu_read_unlock();

	get_mac_from_mdb_entry(entry, &mac_hi, &mac_lo);
	index = mcast_entry_get(entry->vid, mac_hi);
	if (index == -1)
		return -1;

	group = &hnat_priv->pmcast->mtbl[index];
	group->mac_hi = mac_hi;
	group->mac_lo = mac_lo;
	switch (type) {
	case RTM_NEWMDB:
		if (IS_LAN(dev) || IS_WAN(dev))
			group->eif++;
		else
			group->oif++;
		group->vid = entry->vid;
		group->valid = true;
		break;
	case RTM_DELMDB:
		if (group->valid) {
			if (IS_LAN(dev) || IS_WAN(dev))
				group->eif--;
			else
				group->oif--;
			}
		break;
	}
	trace_printk("%s:devname=%s,eif=%d,oif=%d\n", __func__,
		     dev->name, group->eif, group->oif);
	if (group->valid) {
		if (group->oif && group->eif)
			/*eth&wifi both in group,forward to cpu&GDMA1*/
			group->mc_port = (MCAST_TO_PDMA || MCAST_TO_GDMA1);
		else if (group->oif)
			/*only wifi in group,forward to cpu only*/
			group->mc_port = MCAST_TO_PDMA;
		else
			/*only eth in group,forward to GDMA1 only*/
			group->mc_port = MCAST_TO_GDMA1;
		if (!group->oif && !group->eif)
			/*nobody in this group,clear the entry*/
			memset(group, 0, sizeof(struct ppe_mcast_group));

		for (i = 0; i < CFG_PPE_NUM; i++)
			set_hnat_mtbl(group, i, index);
	}

	return 0;
}

static void hnat_mcast_nlmsg_handler(struct work_struct *work)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	struct nlattr *nest, *nest2, *info;
	struct br_port_msg *bpm;
	struct br_mdb_entry *entry;
	struct ppe_mcast_table *pmcast;
	struct sock *sk;

	pmcast = container_of(work, struct ppe_mcast_table, work);
	sk = pmcast->msock->sk;

	while ((skb = skb_dequeue(&sk->sk_receive_queue))) {
		nlh = nlmsg_hdr(skb);
		if (!nlmsg_ok(nlh, skb->len)) {
			kfree_skb(skb);
			continue;
		}
		bpm = nlmsg_data(nlh);
		nest = nlmsg_find_attr(nlh, sizeof(bpm), MDBA_MDB);
		if (!nest) {
			kfree_skb(skb);
			continue;
		}
		nest2 = nla_find_nested(nest, MDBA_MDB_ENTRY);
		if (nest2) {
			info = nla_find_nested(nest2, MDBA_MDB_ENTRY_INFO);
			if (!info) {
				kfree_skb(skb);
				continue;
			}

			entry = (struct br_mdb_entry *)nla_data(info);
			trace_printk("%s:cmd=0x%2x,ifindex=0x%x,state=0x%x",
				     __func__, nlh->nlmsg_type,
				     entry->ifindex, entry->state);
			trace_printk("vid=0x%x,ip=0x%x,proto=0x%x\n",
				     entry->vid, entry->addr.u.ip4,
				     entry->addr.proto);
			hnat_mcast_table_update(nlh->nlmsg_type, entry);
		}
		kfree_skb(skb);
	}
}

static void hnat_mcast_nlmsg_rcv(struct sock *sk)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;
	struct workqueue_struct *queue = pmcast->queue;
	struct work_struct *work = &pmcast->work;

	queue_work(queue, work);
}

static struct socket *hnat_mcast_netlink_open(struct net *net)
{
	struct socket *sock = NULL;
	int ret;
	struct sockaddr_nl addr;

	ret = sock_create_kern(net, PF_NETLINK, SOCK_RAW, NETLINK_ROUTE, &sock);
	if (ret < 0)
		goto out;

	sock->sk->sk_data_ready = hnat_mcast_nlmsg_rcv;
	addr.nl_family = PF_NETLINK;
	addr.nl_pid = 65536; /*fix me:how to get an unique id?*/
	addr.nl_groups = RTMGRP_MDB;
	ret = sock->ops->bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0)
		goto out;

	return sock;
out:
	if (sock)
		sock_release(sock);

	return NULL;
}

static void hnat_mcast_check_timestamp(struct timer_list *t)
{
	struct foe_entry *entry;
	int i, hash_index;
	u16 e_ts, foe_ts;

	for (i = 0; i < CFG_PPE_NUM; i++) {
		for (hash_index = 0; hash_index < hnat_priv->foe_etry_num; hash_index++) {
			entry = hnat_priv->foe_table_cpu[i] + hash_index;
			if (entry->bfib1.sta == 1) {
				e_ts = (entry->ipv4_hnapt.m_timestamp) & 0xffff;
				foe_ts = foe_timestamp(hnat_priv);
				if ((foe_ts - e_ts) > 0x3000)
					foe_ts = (~(foe_ts)) & 0xffff;
				if (abs(foe_ts - e_ts) > 20)
					entry_delete(i, hash_index);
			}
		}
	}
	mod_timer(&hnat_priv->hnat_mcast_check_timer, jiffies + 10 * HZ);
}

int hnat_mcast_enable(u32 ppe_id)
{
	struct ppe_mcast_table *pmcast;

	if (ppe_id >= CFG_PPE_NUM)
		return -EINVAL;

	pmcast = kzalloc(sizeof(*pmcast), GFP_KERNEL);
	if (!pmcast)
		return -1;

	if (hnat_priv->data->version == MTK_HNAT_V1)
		pmcast->max_entry = 0x10;
	else
		pmcast->max_entry = MAX_MCAST_ENTRY;

	INIT_WORK(&pmcast->work, hnat_mcast_nlmsg_handler);
	pmcast->queue = create_singlethread_workqueue("ppe_mcast");
	if (!pmcast->queue)
		goto err;

	pmcast->msock = hnat_mcast_netlink_open(&init_net);
	if (!pmcast->msock)
		goto err;

	hnat_priv->pmcast = pmcast;

	/* mt7629 should checkout mcast entry life time manualy */
	if (hnat_priv->data->version == MTK_HNAT_V3) {
		timer_setup(&hnat_priv->hnat_mcast_check_timer,
			    hnat_mcast_check_timestamp, 0);
		hnat_priv->hnat_mcast_check_timer.expires = jiffies;
		add_timer(&hnat_priv->hnat_mcast_check_timer);
	}

	/* Enable multicast table lookup */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_GLO_CFG, MCAST_TB_EN, 1);
	/* multicast port0 map to PDMA */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P0_PPSE, 0);
	/* multicast port1 map to GMAC1 */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P1_PPSE, 1);
	/* multicast port2 map to GMAC2 */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P2_PPSE, 2);
	/* multicast port3 map to QDMA */
	cr_set_field(hnat_priv->ppe_base[ppe_id] + PPE_MCAST_PPSE, MC_P3_PPSE, 5);

	return 0;
err:
	if (pmcast->queue)
		destroy_workqueue(pmcast->queue);
	if (pmcast->msock)
		sock_release(pmcast->msock);
	kfree(pmcast);

	return -1;
}

int hnat_mcast_disable(void)
{
	struct ppe_mcast_table *pmcast = hnat_priv->pmcast;

	if (!pmcast)
		return -EINVAL;

	if (hnat_priv->data->version == MTK_HNAT_V3)
		del_timer_sync(&hnat_priv->hnat_mcast_check_timer);

	flush_work(&pmcast->work);
	destroy_workqueue(pmcast->queue);
	sock_release(pmcast->msock);
	kfree(pmcast);

	return 0;
}
