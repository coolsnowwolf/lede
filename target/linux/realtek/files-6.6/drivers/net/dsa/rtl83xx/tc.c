// SPDX-License-Identifier: GPL-2.0-only

#include <net/dsa.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <net/flow_offload.h>
#include <linux/rhashtable.h>
#include <asm/mach-rtl838x/mach-rtl83xx.h>

#include "rtl83xx.h"
#include "rtl838x.h"

/* Parse the flow rule for the matching conditions */
static int rtl83xx_parse_flow_rule(struct rtl838x_switch_priv *priv,
			      struct flow_rule *rule, struct rtl83xx_flow *flow)
{
	struct flow_dissector *dissector = rule->match.dissector;

	pr_debug("In %s\n", __func__);
	/* KEY_CONTROL and KEY_BASIC are needed for forming a meaningful key */
	if ((dissector->used_keys & BIT(FLOW_DISSECTOR_KEY_CONTROL)) == 0 ||
	    (dissector->used_keys & BIT(FLOW_DISSECTOR_KEY_BASIC)) == 0) {
		pr_err("Cannot form TC key: used_keys = 0x%llx\n", dissector->used_keys);
		return -EOPNOTSUPP;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_BASIC)) {
		struct flow_match_basic match;

		pr_debug("%s: BASIC\n", __func__);
		flow_rule_match_basic(rule, &match);
		if (match.key->n_proto == htons(ETH_P_ARP))
			flow->rule.frame_type = 0;
		if (match.key->n_proto == htons(ETH_P_IP))
			flow->rule.frame_type = 2;
		if (match.key->n_proto == htons(ETH_P_IPV6))
			flow->rule.frame_type = 3;
		if ((match.key->n_proto == htons(ETH_P_ARP)) || flow->rule.frame_type)
			flow->rule.frame_type_m = 3;
		if (flow->rule.frame_type >= 2) {
			if (match.key->ip_proto == IPPROTO_UDP)
				flow->rule.frame_type_l4 = 0;
			if (match.key->ip_proto == IPPROTO_TCP)
				flow->rule.frame_type_l4 = 1;
			if (match.key->ip_proto == IPPROTO_ICMP || match.key->ip_proto == IPPROTO_ICMPV6)
				flow->rule.frame_type_l4 = 2;
			if (match.key->ip_proto == IPPROTO_TCP)
				flow->rule.frame_type_l4 = 3;
			if ((match.key->ip_proto == IPPROTO_UDP) || flow->rule.frame_type_l4)
				flow->rule.frame_type_l4_m = 7;
		}
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_ETH_ADDRS)) {
		struct flow_match_eth_addrs match;

		pr_debug("%s: ETH_ADDR\n", __func__);
		flow_rule_match_eth_addrs(rule, &match);
		ether_addr_copy(flow->rule.dmac, match.key->dst);
		ether_addr_copy(flow->rule.dmac_m, match.mask->dst);
		ether_addr_copy(flow->rule.smac, match.key->src);
		ether_addr_copy(flow->rule.smac_m, match.mask->src);
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_VLAN)) {
		struct flow_match_vlan match;

		pr_debug("%s: VLAN\n", __func__);
		flow_rule_match_vlan(rule, &match);
		flow->rule.itag = match.key->vlan_id;
		flow->rule.itag_m = match.mask->vlan_id;
		/* TODO: What about match.key->vlan_priority? */
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV4_ADDRS)) {
		struct flow_match_ipv4_addrs match;

		pr_debug("%s: IPV4\n", __func__);
		flow_rule_match_ipv4_addrs(rule, &match);
		flow->rule.is_ipv6 = false;
		flow->rule.dip = match.key->dst;
		flow->rule.dip_m = match.mask->dst;
		flow->rule.sip = match.key->src;
		flow->rule.sip_m = match.mask->src;
	} else if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_IPV6_ADDRS)) {
		struct flow_match_ipv6_addrs match;

		pr_debug("%s: IPV6\n", __func__);
		flow->rule.is_ipv6 = true;
		flow_rule_match_ipv6_addrs(rule, &match);
		flow->rule.dip6 = match.key->dst;
		flow->rule.dip6_m = match.mask->dst;
		flow->rule.sip6 = match.key->src;
		flow->rule.sip6_m = match.mask->src;
	}

	if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_PORTS)) {
		struct flow_match_ports match;

		pr_debug("%s: PORTS\n", __func__);
		flow_rule_match_ports(rule, &match);
		flow->rule.dport = match.key->dst;
		flow->rule.dport_m = match.mask->dst;
		flow->rule.sport = match.key->src;
		flow->rule.sport_m = match.mask->src;
	}

	/* TODO: ICMP */
	return 0;
}

static void rtl83xx_flow_bypass_all(struct rtl83xx_flow *flow)
{
	flow->rule.bypass_sel = true;
	flow->rule.bypass_all = true;
	flow->rule.bypass_igr_stp = true;
	flow->rule.bypass_ibc_sc = true;
}

static int rtl83xx_parse_fwd(struct rtl838x_switch_priv *priv,
			     const struct flow_action_entry *act, struct rtl83xx_flow *flow)
{
	struct net_device *dev = act->dev;
	int port;

	port = rtl83xx_port_is_under(dev, priv);
	if (port < 0) {
		netdev_info(dev, "%s: not a DSA device.\n", __func__);
		return -EINVAL;
	}

	flow->rule.fwd_sel = true;
	flow->rule.fwd_data = port;
	pr_debug("Using port index: %d\n", port);
	rtl83xx_flow_bypass_all(flow);

	return 0;
}

static int rtl83xx_add_flow(struct rtl838x_switch_priv *priv, struct flow_cls_offload *f,
			    struct rtl83xx_flow *flow)
{
	struct flow_rule *rule = flow_cls_offload_flow_rule(f);
	const struct flow_action_entry *act;
	int i, err;

	pr_debug("%s\n", __func__);

	rtl83xx_parse_flow_rule(priv, rule, flow);

	flow_action_for_each(i, act, &rule->action) {
		switch (act->id) {
		case FLOW_ACTION_DROP:
			pr_debug("%s: DROP\n", __func__);
			flow->rule.drop = true;
			rtl83xx_flow_bypass_all(flow);
			return 0;

		case FLOW_ACTION_TRAP:
			pr_debug("%s: TRAP\n", __func__);
			flow->rule.fwd_data = priv->cpu_port;
			flow->rule.fwd_act = PIE_ACT_REDIRECT_TO_PORT;
			rtl83xx_flow_bypass_all(flow);
			break;

		case FLOW_ACTION_MANGLE:
			pr_err("%s: FLOW_ACTION_MANGLE not supported\n", __func__);
			return -EOPNOTSUPP;

		case FLOW_ACTION_ADD:
			pr_err("%s: FLOW_ACTION_ADD not supported\n", __func__);
			return -EOPNOTSUPP;

		case FLOW_ACTION_VLAN_PUSH:
			pr_debug("%s: VLAN_PUSH\n", __func__);
/*			TODO: act->vlan.proto */
			flow->rule.ivid_act = PIE_ACT_VID_ASSIGN;
			flow->rule.ivid_sel = true;
			flow->rule.ivid_data = htons(act->vlan.vid);
			flow->rule.ovid_act = PIE_ACT_VID_ASSIGN;
			flow->rule.ovid_sel = true;
			flow->rule.ovid_data = htons(act->vlan.vid);
			flow->rule.fwd_mod_to_cpu = true;
			break;

		case FLOW_ACTION_VLAN_POP:
			pr_debug("%s: VLAN_POP\n", __func__);
			flow->rule.ivid_act = PIE_ACT_VID_ASSIGN;
			flow->rule.ivid_data = 0;
			flow->rule.ivid_sel = true;
			flow->rule.ovid_act = PIE_ACT_VID_ASSIGN;
			flow->rule.ovid_data = 0;
			flow->rule.ovid_sel = true;
			flow->rule.fwd_mod_to_cpu = true;
			break;

		case FLOW_ACTION_CSUM:
			pr_err("%s: FLOW_ACTION_CSUM not supported\n", __func__);
			return -EOPNOTSUPP;

		case FLOW_ACTION_REDIRECT:
			pr_debug("%s: REDIRECT\n", __func__);
			err = rtl83xx_parse_fwd(priv, act, flow);
			if (err)
				return err;
			flow->rule.fwd_act = PIE_ACT_REDIRECT_TO_PORT;
			break;

		case FLOW_ACTION_MIRRED:
			pr_debug("%s: MIRRED\n", __func__);
			err = rtl83xx_parse_fwd(priv, act, flow);
			if (err)
				return err;
			flow->rule.fwd_act = PIE_ACT_COPY_TO_PORT;
			break;

		default:
			pr_err("%s: Flow action not supported: %d\n", __func__, act->id);
			return -EOPNOTSUPP;
		}
	}

	return 0;
}

static const struct rhashtable_params tc_ht_params = {
	.head_offset = offsetof(struct rtl83xx_flow, node),
	.key_offset = offsetof(struct rtl83xx_flow, cookie),
	.key_len = sizeof(((struct rtl83xx_flow *)0)->cookie),
	.automatic_shrinking = true,
};

static int rtl83xx_configure_flower(struct rtl838x_switch_priv *priv,
				    struct flow_cls_offload *f)
{
	struct rtl83xx_flow *flow;
	int err = 0;

	pr_debug("In %s\n", __func__);

	rcu_read_lock();
	pr_debug("Cookie %08lx\n", f->cookie);
	flow = rhashtable_lookup(&priv->tc_ht, &f->cookie, tc_ht_params);
	if (flow) {
		pr_info("%s: Got flow\n", __func__);
		err = -EEXIST;
		goto rcu_unlock;
	}

rcu_unlock:
	rcu_read_unlock();
	if (flow)
		goto out;
	pr_debug("%s: New flow\n", __func__);

	flow = kzalloc(sizeof(*flow), GFP_KERNEL);
	if (!flow) {
		err = -ENOMEM;
		goto out;
	}

	flow->cookie = f->cookie;
	flow->priv = priv;

	err = rhashtable_insert_fast(&priv->tc_ht, &flow->node, tc_ht_params);
	if (err) {
		pr_err("Could not insert add new rule\n");
		goto out_free;
	}

	rtl83xx_add_flow(priv, f, flow); /* TODO: check error */

	/* Add log action to flow */
	flow->rule.packet_cntr = rtl83xx_packet_cntr_alloc(priv);
	if (flow->rule.packet_cntr >= 0) {
		pr_debug("Using packet counter %d\n", flow->rule.packet_cntr);
		flow->rule.log_sel = true;
		flow->rule.log_data = flow->rule.packet_cntr;
	}

	err = priv->r->pie_rule_add(priv, &flow->rule);
	return err;

out_free:
	kfree(flow);
out:
	pr_err("%s: error %d\n", __func__, err);

	return err;
}

static int rtl83xx_delete_flower(struct rtl838x_switch_priv *priv,
				 struct flow_cls_offload * cls_flower)
{
	struct rtl83xx_flow *flow;

	pr_debug("In %s\n", __func__);
	rcu_read_lock();
	flow = rhashtable_lookup_fast(&priv->tc_ht, &cls_flower->cookie, tc_ht_params);
	if (!flow) {
		rcu_read_unlock();
		return -EINVAL;
	}

	priv->r->pie_rule_rm(priv, &flow->rule);

	rhashtable_remove_fast(&priv->tc_ht, &flow->node, tc_ht_params);

	kfree_rcu(flow, rcu_head);

	rcu_read_unlock();

	return 0;
}

static int rtl83xx_stats_flower(struct rtl838x_switch_priv *priv,
				struct flow_cls_offload * cls_flower)
{
	struct rtl83xx_flow *flow;
	unsigned long lastused = 0;
	int total_packets, new_packets;

	pr_debug("%s: \n", __func__);
	flow = rhashtable_lookup_fast(&priv->tc_ht, &cls_flower->cookie, tc_ht_params);
	if (!flow)
		return -1;

	if (flow->rule.packet_cntr >= 0) {
		total_packets = priv->r->packet_cntr_read(flow->rule.packet_cntr);
		pr_debug("Total packets: %d\n", total_packets);
		new_packets = total_packets - flow->rule.last_packet_cnt;
		flow->rule.last_packet_cnt = total_packets;
	}

	/* TODO: We need a second PIE rule to count the bytes */
	flow_stats_update(&cls_flower->stats, 100 * new_packets, new_packets, 0, lastused,
	                  FLOW_ACTION_HW_STATS_IMMEDIATE);

	return 0;
}

static int rtl83xx_setup_tc_cls_flower(struct rtl838x_switch_priv *priv,
				       struct flow_cls_offload *cls_flower)
{
	pr_debug("%s: %d\n", __func__, cls_flower->command);
	switch (cls_flower->command) {
	case FLOW_CLS_REPLACE:
		return rtl83xx_configure_flower(priv, cls_flower);
	case FLOW_CLS_DESTROY:
		return rtl83xx_delete_flower(priv, cls_flower);
	case FLOW_CLS_STATS:
		return rtl83xx_stats_flower(priv, cls_flower);
	default:
		return -EOPNOTSUPP;
	}
}


static int rtl83xx_setup_tc_block_cb(enum tc_setup_type type, void *type_data,
				     void *cb_priv)
{
	struct rtl838x_switch_priv *priv = cb_priv;

	switch (type) {
	case TC_SETUP_CLSFLOWER:
		pr_debug("%s: TC_SETUP_CLSFLOWER\n", __func__);
		return rtl83xx_setup_tc_cls_flower(priv, type_data);
	default:
		return -EOPNOTSUPP;
	}
}

static LIST_HEAD(rtl83xx_block_cb_list);

int rtl83xx_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
	struct rtl838x_switch_priv *priv;
	struct flow_block_offload *f = type_data;
	static bool first_time = true;
	int err;

	pr_debug("%s: %d\n", __func__, type);

	if(!netdev_uses_dsa(dev)) {
		pr_err("%s: no DSA\n", __func__);
		return 0;
	}
	priv = dev->dsa_ptr->ds->priv;

	switch (type) {
	case TC_SETUP_BLOCK:
		if (first_time) {
			first_time = false;
			err = rhashtable_init(&priv->tc_ht, &tc_ht_params);
			if (err)
				pr_err("%s: Could not initialize hash table\n", __func__);
		}

		f->unlocked_driver_cb = true;
		return flow_block_cb_setup_simple(type_data,
						  &rtl83xx_block_cb_list,
						  rtl83xx_setup_tc_block_cb,
						  priv, priv, true);
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}
