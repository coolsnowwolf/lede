// SPDX-License-Identifier: GPL-2.0-only
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include "phytmac.h"
#include "phytmac_v1.h"
#include "phytmac_v2.h"
#include "phytmac_ptp.h"

static void phytmac_get_ethtool_stats(struct net_device *ndev,
				      struct ethtool_stats *stats,
				      u64 *data)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	int i = PHYTMAC_STATS_LEN, j;
	int q;
	struct phytmac_queue *queue;
	unsigned long *stat;

	hw_if->get_stats(pdata);

	for (q = 0, queue = pdata->queues; q < pdata->queues_num; ++q, ++queue)
		for (j = 0, stat = &queue->stats.rx_packets; j < QUEUE_STATS_LEN; ++j, ++stat)
			pdata->ethtool_stats[i++] = *stat;

	memcpy(data, &pdata->ethtool_stats, sizeof(u64)
			* (PHYTMAC_STATS_LEN + QUEUE_STATS_LEN * pdata->queues_num));
}

static inline int phytmac_get_sset_count(struct net_device *ndev, int sset)
{
	struct phytmac *pdata = netdev_priv(ndev);

	switch (sset) {
	case ETH_SS_STATS:
		return PHYTMAC_STATS_LEN + QUEUE_STATS_LEN * pdata->queues_num;
	default:
		return -EOPNOTSUPP;
	}
}

static void phytmac_get_ethtool_strings(struct net_device *ndev, u32 sset, u8 *p)
{
	char stat_string[ETH_GSTRING_LEN];
	struct phytmac *pdata = netdev_priv(ndev);
	struct phytmac_queue *queue;
	unsigned int i;
	unsigned int q;

	switch (sset) {
	case ETH_SS_STATS:
		for (i = 0; i < PHYTMAC_STATS_LEN; i++, p += ETH_GSTRING_LEN)
			memcpy(p, phytmac_statistics[i].stat_string,
			       ETH_GSTRING_LEN);

		for (q = 0, queue = pdata->queues; q < pdata->queues_num; ++q, ++queue) {
			for (i = 0; i < QUEUE_STATS_LEN; i++, p += ETH_GSTRING_LEN) {
				snprintf(stat_string, ETH_GSTRING_LEN, "q%d_%s",
					 q, queue_statistics[i].stat_string);
				memcpy(p, stat_string, ETH_GSTRING_LEN);
			}
		}
		break;
	}
}

static inline int phytmac_get_regs_len(struct net_device *ndev)
{
	return PHYTMAC_ETHTOOLD_REGS_LEN;
}

static void phytmac_get_regs(struct net_device *ndev,
			     struct ethtool_regs *regs,
			     void *p)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	u32 *regs_buff = p;

	memset(p, 0, PHYTMAC_ETHTOOLD_REGS_LEN);

	hw_if->get_regs(pdata, regs_buff);
}

static void phytmac_get_wol(struct net_device *ndev, struct ethtool_wolinfo *wol)
{
	struct phytmac *pdata = netdev_priv(ndev);

	wol->wolopts = 0;
	phylink_ethtool_get_wol(pdata->phylink, wol);

	wol->supported = WAKE_MAGIC | WAKE_ARP |
			 WAKE_UCAST | WAKE_MCAST;

	if (pdata->wol & PHYTMAC_WAKE_MAGIC)
		wol->wolopts |= WAKE_MAGIC;
	if (pdata->wol & PHYTMAC_WAKE_ARP)
		wol->wolopts |= WAKE_ARP;
	if (pdata->wol & PHYTMAC_WAKE_UCAST)
		wol->wolopts |= WAKE_UCAST;
	if (pdata->wol & PHYTMAC_WAKE_MCAST)
		wol->wolopts |= WAKE_MCAST;
}

static int phytmac_set_wol(struct net_device *ndev, struct ethtool_wolinfo *wol)
{
	struct phytmac *pdata = netdev_priv(ndev);
	int ret;

	ret = phylink_ethtool_set_wol(pdata->phylink, wol);

	/* Don't manage WoL on MAC, if PHY set_wol() fails */
	if (ret && ret != -EOPNOTSUPP)
		return ret;

	pdata->wol = 0;

	if (wol->wolopts & WAKE_MAGIC)
		pdata->wol |= PHYTMAC_WAKE_MAGIC;
	if (wol->wolopts & WAKE_ARP)
		pdata->wol |= PHYTMAC_WAKE_ARP;
	if (wol->wolopts & WAKE_UCAST)
		pdata->wol |= PHYTMAC_WAKE_UCAST;
	if (wol->wolopts & WAKE_MCAST)
		pdata->wol |= PHYTMAC_WAKE_MCAST;

	device_set_wakeup_enable(pdata->dev, pdata->wol ? 1 : 0);
	phytmac_set_bios_wol_enable(pdata, pdata->wol ? 1 : 0);

	return 0;
}

static void phytmac_get_ringparam(struct net_device *ndev,
				  struct ethtool_ringparam *ring,
				  struct kernel_ethtool_ringparam *kernel_ring,
				  struct netlink_ext_ack *extack)
{
	struct phytmac *pdata = netdev_priv(ndev);

	ring->rx_max_pending = MAX_RX_RING_SIZE;
	ring->tx_max_pending = MAX_TX_RING_SIZE;

	ring->rx_pending = pdata->rx_ring_size;
	ring->tx_pending = pdata->tx_ring_size;
}

static int phytmac_set_ringparam(struct net_device *ndev,
				 struct ethtool_ringparam *ring,
				 struct kernel_ethtool_ringparam *kernel_ring,
				 struct netlink_ext_ack *extack)
{
	struct phytmac *pdata = netdev_priv(ndev);
	u32 new_rx_size, new_tx_size;

	if (ring->rx_mini_pending || ring->rx_jumbo_pending)
		return -EINVAL;

	new_rx_size = clamp_t(u32, ring->rx_pending,
			      MIN_RX_RING_SIZE, MAX_RX_RING_SIZE);
	new_rx_size = roundup_pow_of_two(new_rx_size);

	new_tx_size = clamp_t(u32, ring->tx_pending,
			      MIN_TX_RING_SIZE, MAX_TX_RING_SIZE);
	new_tx_size = roundup_pow_of_two(new_tx_size);

	if (EQUAL(new_tx_size, pdata->tx_ring_size) &&
	    EQUAL(new_rx_size, pdata->rx_ring_size)) {
		/* nothing to do */
		return 0;
	}

	return phytmac_reset_ringsize(pdata, new_rx_size, new_tx_size);
}

static int phytmac_get_ts_info(struct net_device *ndev,
			       struct kernel_ethtool_ts_info *info)
{
	struct phytmac *pdata = netdev_priv(ndev);

	if (IS_REACHABLE(CONFIG_PHYTMAC_ENABLE_PTP)) {
		info->so_timestamping =
			SOF_TIMESTAMPING_TX_SOFTWARE |
			SOF_TIMESTAMPING_RX_SOFTWARE |
			SOF_TIMESTAMPING_SOFTWARE |
			SOF_TIMESTAMPING_TX_HARDWARE |
			SOF_TIMESTAMPING_RX_HARDWARE |
			SOF_TIMESTAMPING_RAW_HARDWARE;
		info->tx_types =
			(1 << HWTSTAMP_TX_ONESTEP_SYNC) |
			(1 << HWTSTAMP_TX_OFF) |
			(1 << HWTSTAMP_TX_ON);
		info->rx_filters =
			(1 << HWTSTAMP_FILTER_NONE) |
			(1 << HWTSTAMP_FILTER_ALL);

		info->phc_index = pdata->ptp_clock ? ptp_clock_index(pdata->ptp_clock) : -1;

		return 0;
	}

	return ethtool_op_get_ts_info(ndev, info);
}

static int phytmac_add_fdir_ethtool(struct net_device *ndev,
				    struct ethtool_rxnfc *cmd)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct ethtool_rx_flow_spec *fs = &cmd->fs;
	struct ethtool_rx_fs_item *item, *newfs;
	unsigned long flags;
	int ret = -EINVAL;
	bool added = false;
	struct phytmac_hw_if *hw_if = pdata->hw_if;

	if (cmd->fs.location >= pdata->max_rx_fs ||
	    cmd->fs.ring_cookie >= pdata->queues_num) {
		return -EINVAL;
	}

	newfs = kmalloc(sizeof(*newfs), GFP_KERNEL);
	if (!newfs)
		return -ENOMEM;
	memcpy(&newfs->fs, fs, sizeof(newfs->fs));

	netdev_dbg(ndev, "Adding flow filter entry,type=%u,queue=%u,loc=%u,src=%08X,dst=%08X,ps=%u,pd=%u\n",
		   fs->flow_type, (int)fs->ring_cookie, fs->location,
		   htonl(fs->h_u.tcp_ip4_spec.ip4src),
		   htonl(fs->h_u.tcp_ip4_spec.ip4dst),
		   htons(fs->h_u.tcp_ip4_spec.psrc), htons(fs->h_u.tcp_ip4_spec.pdst));

	spin_lock_irqsave(&pdata->rx_fs_lock, flags);

	/* find correct place to add in list */
	list_for_each_entry(item, &pdata->rx_fs_list.list, list) {
		if (item->fs.location > newfs->fs.location) {
			list_add_tail(&newfs->list, &item->list);
			added = true;
			break;
		} else if (item->fs.location == fs->location) {
			netdev_err(ndev, "Rule not added: location %d not free!\n",
				   fs->location);
			ret = -EBUSY;
			goto err;
		}
	}
	if (!added)
		list_add_tail(&newfs->list, &pdata->rx_fs_list.list);

	hw_if->add_fdir_entry(pdata, fs);
	pdata->rx_fs_list.count++;

	spin_unlock_irqrestore(&pdata->rx_fs_lock, flags);
	return 0;

err:
	spin_unlock_irqrestore(&pdata->rx_fs_lock, flags);
	kfree(newfs);
	return ret;
}

static int phytmac_del_fdir_ethtool(struct net_device *ndev,
				    struct ethtool_rxnfc *cmd)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct ethtool_rx_fs_item *item;
	struct ethtool_rx_flow_spec *fs;
	unsigned long flags;
	struct phytmac_hw_if *hw_if = pdata->hw_if;

	spin_lock_irqsave(&pdata->rx_fs_lock, flags);

	list_for_each_entry(item, &pdata->rx_fs_list.list, list) {
		if (item->fs.location == cmd->fs.location) {
			/* disable screener regs for the flow entry */
			fs = &item->fs;
			netdev_dbg(ndev, "Deleting flow filter entry,type=%u,queue=%u,loc=%u,src=%08X,dst=%08X,ps=%u,pd=%u\n",
				   fs->flow_type, (int)fs->ring_cookie, fs->location,
				   htonl(fs->h_u.tcp_ip4_spec.ip4src),
				   htonl(fs->h_u.tcp_ip4_spec.ip4dst),
				   htons(fs->h_u.tcp_ip4_spec.psrc),
				   htons(fs->h_u.tcp_ip4_spec.pdst));

			hw_if->del_fdir_entry(pdata, fs);

			list_del(&item->list);
			pdata->rx_fs_list.count--;
			spin_unlock_irqrestore(&pdata->rx_fs_lock, flags);
			kfree(item);
			return 0;
		}
	}

	spin_unlock_irqrestore(&pdata->rx_fs_lock, flags);
	return -EINVAL;
}

static int phytmac_get_fdir_entry(struct net_device *ndev,
				  struct ethtool_rxnfc *cmd)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct ethtool_rx_fs_item *item;

	list_for_each_entry(item, &pdata->rx_fs_list.list, list) {
		if (item->fs.location == cmd->fs.location) {
			memcpy(&cmd->fs, &item->fs, sizeof(cmd->fs));
			return 0;
		}
	}
	return -EINVAL;
}

static int phytmac_get_all_fdir_entries(struct net_device *ndev,
					struct ethtool_rxnfc *cmd,
					u32 *rule_locs)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct ethtool_rx_fs_item *item;
	u32 cnt = 0;

	list_for_each_entry(item, &pdata->rx_fs_list.list, list) {
		if (cnt == cmd->rule_cnt)
			return -EMSGSIZE;
		rule_locs[cnt] = item->fs.location;
		cnt++;
	}
	cmd->data = pdata->max_rx_fs;
	cmd->rule_cnt = cnt;

	return 0;
}

static int phytmac_get_rxnfc(struct net_device *ndev,
			     struct ethtool_rxnfc *cmd,
			     u32 *rule_locs)
{
	struct phytmac *pdata = netdev_priv(ndev);
	int ret = 0;

	switch (cmd->cmd) {
	case ETHTOOL_GRXRINGS:
		cmd->data = pdata->queues_num;
		break;
	case ETHTOOL_GRXCLSRLCNT:
		cmd->rule_cnt = pdata->rx_fs_list.count;
		break;
	case ETHTOOL_GRXCLSRULE:
		ret = phytmac_get_fdir_entry(ndev, cmd);
		break;
	case ETHTOOL_GRXCLSRLALL:
		ret = phytmac_get_all_fdir_entries(ndev, cmd, rule_locs);
		break;
	default:
		netdev_err(ndev, "Command parameter %d is not supported\n", cmd->cmd);
		ret = -EOPNOTSUPP;
	}

	return ret;
}

static int phytmac_set_rxnfc(struct net_device *ndev, struct ethtool_rxnfc *cmd)
{
	switch (cmd->cmd) {
	case ETHTOOL_SRXCLSRLINS:
		return phytmac_add_fdir_ethtool(ndev, cmd);
	case ETHTOOL_SRXCLSRLDEL:
		return phytmac_del_fdir_ethtool(ndev, cmd);
	default:
		netdev_err(ndev, "Command parameter %d is not supported\n", cmd->cmd);
		return -EOPNOTSUPP;
	}
}

static int phytmac_get_link_ksettings(struct net_device *ndev,
				      struct ethtool_link_ksettings *kset)
{
	int ret = 0;
	struct phytmac *pdata = netdev_priv(ndev);
	u32 supported = 0;
	u32 advertising = 0;

	if (!ndev->phydev) {
		kset->base.port = PORT_FIBRE;
		kset->base.transceiver = XCVR_INTERNAL;
		if (netif_carrier_ok(ndev)) {
			kset->base.duplex = pdata->duplex;
			kset->base.speed = pdata->speed;
		} else {
			kset->base.duplex = DUPLEX_UNKNOWN;
			kset->base.speed = SPEED_UNKNOWN;
		}

		if (pdata->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
		    pdata->phy_interface == PHY_INTERFACE_MODE_10GBASER) {
			supported = SUPPORTED_10000baseT_Full
				    | SUPPORTED_FIBRE | SUPPORTED_Pause;
			advertising = ADVERTISED_10000baseT_Full
				      | ADVERTISED_FIBRE | ADVERTISED_Pause;
		}  else if (pdata->phy_interface == PHY_INTERFACE_MODE_2500BASEX) {
			supported = SUPPORTED_2500baseX_Full
				    | SUPPORTED_FIBRE | SUPPORTED_Pause;
			advertising = ADVERTISED_2500baseX_Full
				      | ADVERTISED_FIBRE | ADVERTISED_Pause;
		} else if (pdata->phy_interface == PHY_INTERFACE_MODE_1000BASEX) {
			supported = SUPPORTED_1000baseT_Full
				    | SUPPORTED_FIBRE | SUPPORTED_Pause;
			advertising = ADVERTISED_1000baseT_Full
				      | ADVERTISED_FIBRE | ADVERTISED_Pause;
		} else if (pdata->phy_interface == PHY_INTERFACE_MODE_SGMII) {
			supported = SUPPORTED_1000baseT_Full
				    | SUPPORTED_FIBRE | SUPPORTED_Pause;
			advertising = ADVERTISED_1000baseT_Full
				      | ADVERTISED_FIBRE | ADVERTISED_Pause;
		}

		ethtool_convert_legacy_u32_to_link_mode(kset->link_modes.supported,
							supported);
		ethtool_convert_legacy_u32_to_link_mode(kset->link_modes.advertising,
							advertising);
	} else {
		phy_ethtool_get_link_ksettings(ndev, kset);
	}

	return ret;
}

static int phytmac_set_link_ksettings(struct net_device *ndev,
				      const struct ethtool_link_ksettings *kset)
{
	int ret = 0;

	if (!ndev->phydev) {
		netdev_err(ndev, "Without a PHY, setting link is not supported\n");
		ret = -EOPNOTSUPP;
	} else {
		phy_ethtool_set_link_ksettings(ndev, kset);
	}

	return ret;
}

static inline void phytmac_get_pauseparam(struct net_device *ndev,
					  struct ethtool_pauseparam *pause)
{
	struct phytmac *pdata = netdev_priv(ndev);

	pause->rx_pause = pdata->pause;
	pause->tx_pause = pdata->pause;
}

static int phytmac_set_pauseparam(struct net_device *ndev,
				  struct ethtool_pauseparam *pause)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct phytmac_hw_if *hw_if = pdata->hw_if;

	if (pause->rx_pause != pdata->pause)
		hw_if->enable_pause(pdata, pause->rx_pause);

	pdata->pause = pause->rx_pause;

	return 0;
}

static inline void phytmac_get_channels(struct net_device *ndev,
					struct ethtool_channels *ch)
{
	struct phytmac *pdata = netdev_priv(ndev);

	ch->max_combined = pdata->queues_max_num;
	ch->combined_count = pdata->queues_num;
}

static int phytmac_set_channels(struct net_device *ndev,
				struct ethtool_channels *ch)
{
	struct phytmac *pdata = netdev_priv(ndev);

	if (netif_running(ndev))
		return -EBUSY;

	if (ch->combined_count > pdata->queues_max_num) {
		netdev_err(ndev, "combined channel count cannot exceed %u\n",
			   ch->combined_count);

		return -EINVAL;
	}

	pdata->queues_num = ch->combined_count;

	return 0;
}

static inline u32 phytmac_get_msglevel(struct net_device *ndev)
{
	struct phytmac *pdata = netdev_priv(ndev);

	return pdata->msg_enable;
}

static inline void phytmac_set_msglevel(struct net_device *ndev, u32 level)
{
	struct phytmac *pdata = netdev_priv(ndev);

	pdata->msg_enable = level;
}

static void phytmac_get_drvinfo(struct net_device *ndev, struct ethtool_drvinfo *drvinfo)
{
	struct phytmac *pdata = netdev_priv(ndev);

	strscpy(drvinfo->version, PHYTMAC_DRIVER_VERSION, sizeof(drvinfo->version));
	strscpy(drvinfo->fw_version, pdata->fw_version, sizeof(drvinfo->fw_version));

	if (pdata->platdev) {
		strscpy(drvinfo->driver, PHYTMAC_PLAT_DRV_NAME, sizeof(drvinfo->driver));
		strscpy(drvinfo->bus_info, pdata->platdev->name, sizeof(drvinfo->bus_info));
	} else if (pdata->pcidev) {
		strscpy(drvinfo->driver, PHYTMAC_PCI_DRV_NAME, sizeof(drvinfo->driver));
		strscpy(drvinfo->bus_info, pci_name(pdata->pcidev), sizeof(drvinfo->bus_info));
	}
}

static const struct ethtool_ops phytmac_ethtool_ops = {
	.get_regs_len			= phytmac_get_regs_len,
	.get_regs			= phytmac_get_regs,
	.get_msglevel			= phytmac_get_msglevel,
	.set_msglevel			= phytmac_set_msglevel,
	.get_link			= ethtool_op_get_link,
	.get_ts_info			= phytmac_get_ts_info,
	.get_ethtool_stats		= phytmac_get_ethtool_stats,
	.get_strings			= phytmac_get_ethtool_strings,
	.get_sset_count			= phytmac_get_sset_count,
	.get_link_ksettings		= phytmac_get_link_ksettings,
	.set_link_ksettings		= phytmac_set_link_ksettings,
	.get_ringparam			= phytmac_get_ringparam,
	.set_ringparam			= phytmac_set_ringparam,
	.get_rxnfc			= phytmac_get_rxnfc,
	.set_rxnfc			= phytmac_set_rxnfc,
	.get_pauseparam			= phytmac_get_pauseparam,
	.set_pauseparam			= phytmac_set_pauseparam,
	.get_channels			= phytmac_get_channels,
	.set_channels			= phytmac_set_channels,
	.get_wol			= phytmac_get_wol,
	.set_wol			= phytmac_set_wol,
	.get_drvinfo			= phytmac_get_drvinfo,
};

void phytmac_set_ethtool_ops(struct net_device *ndev)
{
	ndev->ethtool_ops = &phytmac_ethtool_ops;
}
