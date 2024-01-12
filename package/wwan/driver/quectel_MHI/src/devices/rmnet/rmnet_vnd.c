/* Copyright (c) 2013-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * RMNET Data virtual network driver
 *
 */

#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <net/pkt_sched.h>
#include "rmnet_config.h"
#include "rmnet_handlers.h"
#include "rmnet_private.h"
#include "rmnet_map.h"
#include "rmnet_vnd.h"

/* RX/TX Fixup */

static void rmnet_vnd_rx_fixup(struct net_device *dev, u32 skb_len)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_pcpu_stats *pcpu_ptr;

	pcpu_ptr = this_cpu_ptr(priv->pcpu_stats);

	u64_stats_update_begin(&pcpu_ptr->syncp);
	pcpu_ptr->stats.rx_pkts++;
	pcpu_ptr->stats.rx_bytes += skb_len;
	u64_stats_update_end(&pcpu_ptr->syncp);
}

static void rmnet_vnd_tx_fixup(struct net_device *dev, u32 skb_len)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_pcpu_stats *pcpu_ptr;

	pcpu_ptr = this_cpu_ptr(priv->pcpu_stats);

	u64_stats_update_begin(&pcpu_ptr->syncp);
	pcpu_ptr->stats.tx_pkts++;
	pcpu_ptr->stats.tx_bytes += skb_len;
	u64_stats_update_end(&pcpu_ptr->syncp);
}

/* Network Device Operations */

static netdev_tx_t rmnet_vnd_start_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
	struct rmnet_priv *priv;

	priv = netdev_priv(dev);
	if (priv->real_dev) {
		rmnet_egress_handler(skb);
	} else {
		this_cpu_inc(priv->pcpu_stats->stats.tx_drops);
		kfree_skb(skb);
	}
	return NETDEV_TX_OK;
}

static int rmnet_vnd_change_mtu(struct net_device *rmnet_dev, int new_mtu)
{
	if (new_mtu < 0 || new_mtu > RMNET_MAX_PACKET_SIZE)
		return -EINVAL;

	rmnet_dev->mtu = new_mtu;
	return 0;
}

static int rmnet_vnd_get_iflink(const struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);

	return priv->real_dev->ifindex;
}

static int rmnet_vnd_init(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	int err;

	priv->pcpu_stats = alloc_percpu(struct rmnet_pcpu_stats);
	if (!priv->pcpu_stats)
		return -ENOMEM;

	err = gro_cells_init(&priv->gro_cells, dev);
	if (err) {
		free_percpu(priv->pcpu_stats);
		return err;
	}

	return 0;
}

static void rmnet_vnd_uninit(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);

	gro_cells_destroy(&priv->gro_cells);
	free_percpu(priv->pcpu_stats);
}

static struct rtnl_link_stats64* rmnet_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *s)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_vnd_stats total_stats;
	struct rmnet_pcpu_stats *pcpu_ptr;
	unsigned int cpu, start;

	memset(&total_stats, 0, sizeof(struct rmnet_vnd_stats));

	for_each_possible_cpu(cpu) {
		pcpu_ptr = per_cpu_ptr(priv->pcpu_stats, cpu);

		do {
			start = u64_stats_fetch_begin_irq(&pcpu_ptr->syncp);
			total_stats.rx_pkts += pcpu_ptr->stats.rx_pkts;
			total_stats.rx_bytes += pcpu_ptr->stats.rx_bytes;
			total_stats.tx_pkts += pcpu_ptr->stats.tx_pkts;
			total_stats.tx_bytes += pcpu_ptr->stats.tx_bytes;
		} while (u64_stats_fetch_retry_irq(&pcpu_ptr->syncp, start));

		total_stats.tx_drops += pcpu_ptr->stats.tx_drops;
	}

	s->rx_packets = total_stats.rx_pkts;
	s->rx_bytes = total_stats.rx_bytes;
	s->tx_packets = total_stats.tx_pkts;
	s->tx_bytes = total_stats.tx_bytes;
	s->tx_dropped = total_stats.tx_drops;

	return s;
}

static const struct net_device_ops rmnet_vnd_ops = {
	.ndo_start_xmit = rmnet_vnd_start_xmit,
	.ndo_change_mtu = rmnet_vnd_change_mtu,
	.ndo_get_iflink = rmnet_vnd_get_iflink,
	//.ndo_add_slave  = rmnet_add_bridge,
	//.ndo_del_slave  = rmnet_del_bridge,
	.ndo_init       = rmnet_vnd_init,
	.ndo_uninit     = rmnet_vnd_uninit,
	.ndo_get_stats64 = rmnet_get_stats64,
};

static const char rmnet_gstrings_stats[][ETH_GSTRING_LEN] = {
	"Checksum ok",
	"Checksum valid bit not set",
	"Checksum validation failed",
	"Checksum error bad buffer",
	"Checksum error bad ip version",
	"Checksum error bad transport",
	"Checksum skipped on ip fragment",
	"Checksum skipped",
	"Checksum computed in software",
	"Checksum computed in hardware",
	"Coalescing packets received",
	"Coalesced packets",
	"Coalescing header NLO errors",
	"Coalescing header pcount errors",
	"Coalescing checksum errors",
	"Coalescing packet reconstructs",
	"Coalescing IP version invalid",
	"Coalescing L4 header invalid",
	"Coalescing close Non-coalescable",
	"Coalescing close L3 mismatch",
	"Coalescing close L4 mismatch",
	"Coalescing close HW NLO limit",
	"Coalescing close HW packet limit",
	"Coalescing close HW byte limit",
	"Coalescing close HW time limit",
	"Coalescing close HW eviction",
	"Coalescing close Coalescable",
	"Coalescing packets over VEID0",
	"Coalescing packets over VEID1",
	"Coalescing packets over VEID2",
	"Coalescing packets over VEID3",
};

static const char rmnet_port_gstrings_stats[][ETH_GSTRING_LEN] = {
	"MAP Cmd last version",
	"MAP Cmd last ep id",
	"MAP Cmd last transaction id",
	"DL header last seen sequence",
	"DL header last seen bytes",
	"DL header last seen packets",
	"DL header last seen flows",
	"DL header pkts received",
	"DL header total bytes received",
	"DL header total pkts received",
	"DL trailer last seen sequence",
	"DL trailer pkts received",
};

static void rmnet_get_strings(struct net_device *dev, u32 stringset, u8 *buf)
{
	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(buf, &rmnet_gstrings_stats,
		       sizeof(rmnet_gstrings_stats));
		memcpy(buf + sizeof(rmnet_gstrings_stats),
		       &rmnet_port_gstrings_stats,
		       sizeof(rmnet_port_gstrings_stats));
		break;
	}
}

static int rmnet_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(rmnet_gstrings_stats) +
		       ARRAY_SIZE(rmnet_port_gstrings_stats);
	default:
		return -EOPNOTSUPP;
	}
}

static void rmnet_get_ethtool_stats(struct net_device *dev,
				    struct ethtool_stats *stats, u64 *data)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_priv_stats *st = &priv->stats;
	struct rmnet_port_priv_stats *stp;
	struct rmnet_port *port;

	port = rmnet_get_port(priv->real_dev);

	if (!data || !port)
		return;

	stp = &port->stats;

	memcpy(data, st, ARRAY_SIZE(rmnet_gstrings_stats) * sizeof(u64));
	memcpy(data + ARRAY_SIZE(rmnet_gstrings_stats), stp,
	       ARRAY_SIZE(rmnet_port_gstrings_stats) * sizeof(u64));
}

static int rmnet_stats_reset(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_port_priv_stats *stp;
	struct rmnet_port *port;

	port = rmnet_get_port(priv->real_dev);
	if (!port)
		return -EINVAL;

	stp = &port->stats;

	memset(stp, 0, sizeof(*stp));
	return 0;
}

static const struct ethtool_ops rmnet_ethtool_ops = {
	.get_ethtool_stats = rmnet_get_ethtool_stats,
	.get_strings = rmnet_get_strings,
	.get_sset_count = rmnet_get_sset_count,
	.nway_reset = rmnet_stats_reset,
};

/* Called by kernel whenever a new rmnet<n> device is created. Sets MTU,
 * flags, ARP type, needed headroom, etc...
 */
void rmnet_vnd_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
	rmnet_dev->mtu = RMNET_DFLT_PACKET_SIZE;
	rmnet_dev->needed_headroom = RMNET_NEEDED_HEADROOM;
	random_ether_addr(rmnet_dev->dev_addr);
	rmnet_dev->tx_queue_len = RMNET_TX_QUEUE_LEN;

	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
	rmnet_dev->type = ARPHRD_RAWIP;
	rmnet_dev->hard_header_len = 0;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

	//rmnet_dev->needs_free_netdev = true;

	rmnet_dev->hw_features = NETIF_F_RXCSUM;
	rmnet_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	//rmnet_dev->hw_features |= NETIF_F_SG;
	//rmnet_dev->hw_features |= NETIF_F_GRO_HW;	
}

/* Exposed API */

static int rmnet_vnd_newlink(u8 id, struct net_device *rmnet_dev,
		      struct rmnet_port *port,
		      struct net_device *real_dev,
		      struct rmnet_endpoint *ep)
{
	struct rmnet_priv *priv = netdev_priv(rmnet_dev);
	struct rmnet_nss_cb *nss_cb;
	int rc;

	if (ep->egress_dev)
		return -EINVAL;

	if (rmnet_get_endpoint(port, id))
		return -EBUSY;

	rmnet_dev->hw_features = NETIF_F_RXCSUM;
	rmnet_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	rmnet_dev->hw_features |= NETIF_F_SG;

	priv->real_dev = real_dev;

	rc = register_netdevice(rmnet_dev);
	if (!rc) {
		ep->egress_dev = rmnet_dev;
		ep->mux_id = id;
		port->nr_rmnet_devs++;

		//rmnet_dev->rtnl_link_ops = &rmnet_link_ops;

		priv->mux_id = id;

		netdev_dbg(rmnet_dev, "rmnet dev created\n");
	}

	nss_cb = rcu_dereference(rmnet_nss_callbacks);
	if (nss_cb) {
		rc = nss_cb->nss_create(rmnet_dev);
		if (rc) {
			/* Log, but don't fail the device creation */
			netdev_err(rmnet_dev, "Device will not use NSS path: %d\n", rc);
			rc = 0;
		} else {
			netdev_dbg(rmnet_dev, "NSS context created\n");
		}
	}

	return rc;
}

static int rmnet_vnd_dellink(u8 id, struct rmnet_port *port,
		      struct rmnet_endpoint *ep)
{
	struct rmnet_nss_cb *nss_cb;

	if (id >= RMNET_MAX_LOGICAL_EP || !ep->egress_dev)
		return -EINVAL;

	if (ep->egress_dev) {
		nss_cb = rcu_dereference(rmnet_nss_callbacks);
		if (nss_cb)
			nss_cb->nss_free(ep->egress_dev);
	}
	ep->egress_dev = NULL;
	port->nr_rmnet_devs--;

	return 0;
}

static int rmnet_vnd_do_flow_control(struct net_device *rmnet_dev, int enable)
{
	netdev_dbg(rmnet_dev, "Setting VND TX queue state to %d\n", enable);
	/* Although we expect similar number of enable/disable
	 * commands, optimize for the disable. That is more
	 * latency sensitive than enable
	 */
	if (unlikely(enable))
		netif_wake_queue(rmnet_dev);
	else
		netif_stop_queue(rmnet_dev);

	return 0;
}
