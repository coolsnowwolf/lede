/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2014-2016 Sean Wang <sean.wang@mediatek.com>
 *   Copyright (C) 2016-2017 John Crispin <blogic@openwrt.org>
 */

#include "mtk_offload.h"

static const char *mtk_foe_entry_state_str[] = {
	"INVALID",
	"UNBIND",
	"BIND",
	"FIN"
};

static const char *mtk_foe_packet_type_str[] = {
	"IPV4_HNAPT",
	"IPV4_HNAT",
	"IPV6_1T_ROUTE",
	"IPV4_DSLITE",
	"IPV6_3T_ROUTE",
	"IPV6_5T_ROUTE",
	"IPV6_6RD",
};

#define IPV4_HNAPT                      0
#define IPV4_HNAT                       1
#define IS_IPV4_HNAPT(x)	(((x)->bfib1.pkt_type == IPV4_HNAPT) ? 1: 0)
static struct mtk_eth *_eth;
#define es(entry)		(mtk_foe_entry_state_str[entry->bfib1.state])
//#define ei(entry, end)		(MTK_PPE_TBL_SZ - (int)(end - entry))
#define ei(entry, end)		(MTK_PPE_ENTRY_CNT - (int)(end - entry))
#define pt(entry)		(mtk_foe_packet_type_str[entry->ipv4_hnapt.bfib1.pkt_type])

static int mtk_ppe_debugfs_foe_show(struct seq_file *m, void *private)
{
	struct mtk_eth *eth = _eth;
	struct mtk_foe_entry *entry, *end;
	int i = 0;

	entry = eth->foe_table;
	end = eth->foe_table + MTK_PPE_ENTRY_CNT;

	while (entry < end) {
		if (IS_IPV4_HNAPT(entry)) {
			__be32 saddr = htonl(entry->ipv4_hnapt.sip);
			__be32 daddr = htonl(entry->ipv4_hnapt.dip);
			__be32 nsaddr = htonl(entry->ipv4_hnapt.new_sip);
			__be32 ndaddr = htonl(entry->ipv4_hnapt.new_dip);
			unsigned char h_dest[ETH_ALEN];
			unsigned char h_source[ETH_ALEN];

			*((u32*) h_source) = swab32(entry->ipv4_hnapt.smac_hi);
			*((u16*) &h_source[4]) = swab16(entry->ipv4_hnapt.smac_lo);
			*((u32*) h_dest) = swab32(entry->ipv4_hnapt.dmac_hi);
			*((u16*) &h_dest[4]) = swab16(entry->ipv4_hnapt.dmac_lo);
			seq_printf(m,
				   "(%x)0x%05x|state=%s|type=%s|"
				   "%pI4:%d->%pI4:%d=>%pI4:%d->%pI4:%d|%pM=>%pM|"
				   "etype=0x%04x|info1=0x%x|info2=0x%x|"
				   "vlan1=%d|vlan2=%d\n",
				   i,
				   ei(entry, end), es(entry), pt(entry),
				   &saddr, entry->ipv4_hnapt.sport,
				   &daddr, entry->ipv4_hnapt.dport,
				   &nsaddr, entry->ipv4_hnapt.new_sport,
				   &ndaddr, entry->ipv4_hnapt.new_dport, h_source,
				   h_dest, ntohs(entry->ipv4_hnapt.etype),
				   entry->ipv4_hnapt.info_blk1,
				   entry->ipv4_hnapt.info_blk2,
				   entry->ipv4_hnapt.vlan1,
				   entry->ipv4_hnapt.vlan2);
		} else
			seq_printf(m, "0x%05x state=%s\n",
				   ei(entry, end), es(entry));
		entry++;
		i++;
	}

	return 0;
}

static int mtk_ppe_debugfs_foe_open(struct inode *inode, struct file *file)
{
	return single_open(file, mtk_ppe_debugfs_foe_show, file->private_data);
}

static const struct file_operations mtk_ppe_debugfs_foe_fops = {
	.open = mtk_ppe_debugfs_foe_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int ra_ppe_debugfs_init(struct mtk_eth *eth)
{
	struct dentry *root;

	_eth = eth;

	root = debugfs_create_dir("mtk_ppe", NULL);
	if (!root)
		return -ENOMEM;

	debugfs_create_file("all_entry", S_IRUGO, root, eth, &mtk_ppe_debugfs_foe_fops);

	return 0;
}
