/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2020 MediaTek Inc.
 * Author: Landen Chao <landen.chao@mediatek.com>
 */

#include <linux/of_device.h>
#include <net/netfilter/nf_flow_table.h>
#include "hnat.h"

u32 hnat_dsa_fill_stag(const struct net_device *netdev,
		       struct foe_entry *entry,
		       struct flow_offload_hw_path *hw_path,
		       u16 eth_proto,
		       int mape)
{
	const struct net_device *ndev;
	const unsigned int *port_reg;
	int port_index;
	u16 sp_tag;

	if (hw_path->flags & FLOW_OFFLOAD_PATH_VLAN)
		ndev = hw_path->dev;
	else
		ndev = netdev;

	port_reg = of_get_property(ndev->dev.of_node, "reg", NULL);
	if (unlikely(!port_reg))
		return -EINVAL;

	port_index = be32_to_cpup(port_reg);
	sp_tag = BIT(port_index);

	if (!entry->bfib1.vlan_layer)
		entry->bfib1.vlan_layer = 1;
	else
		/* VLAN existence indicator */
		sp_tag |= BIT(8);
	entry->bfib1.vpm = 0;

	switch (eth_proto) {
	case ETH_P_IP:
		if (entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE
			|| (entry->ipv4_hnapt.bfib1.pkt_type == IPV4_MAP_E))
			entry->ipv4_dslite.etype = sp_tag;
		else
			entry->ipv4_hnapt.etype = sp_tag;
		break;
	case ETH_P_IPV6:
		/* In the case MAPE LAN --> WAN, binding entry is to CPU.
		 * Do not add special tag.
		 */
		if (!mape)
			/* etype offset of ipv6 entries are the same. */
			entry->ipv6_5t_route.etype = sp_tag;

		break;
	default:
		pr_info("DSA + HNAT unsupport protocol\n");
	}

	return port_index;
}
