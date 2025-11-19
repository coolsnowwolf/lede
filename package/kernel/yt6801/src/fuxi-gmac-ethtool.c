// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "fuxi-gmac.h"
#include "fuxi-gmac-reg.h"

struct fxgmac_stats_desc {
    char stat_string[ETH_GSTRING_LEN];
    int stat_offset;
};

#define FXGMAC_STAT(str, var)					\
	{							\
		str,						\
		offsetof(struct fxgmac_pdata, stats.var),	\
	}

static const struct fxgmac_stats_desc fxgmac_gstring_stats[] = {
    /* MMC TX counters */
    FXGMAC_STAT("tx_bytes", txoctetcount_gb),
    FXGMAC_STAT("tx_bytes_good", txoctetcount_g),
    FXGMAC_STAT("tx_packets", txframecount_gb),
    FXGMAC_STAT("tx_packets_good", txframecount_g),
    FXGMAC_STAT("tx_unicast_packets", txunicastframes_gb),
    FXGMAC_STAT("tx_broadcast_packets", txbroadcastframes_gb),
    FXGMAC_STAT("tx_broadcast_packets_good", txbroadcastframes_g),
    FXGMAC_STAT("tx_multicast_packets", txmulticastframes_gb),
    FXGMAC_STAT("tx_multicast_packets_good", txmulticastframes_g),
    FXGMAC_STAT("tx_vlan_packets_good", txvlanframes_g),
    FXGMAC_STAT("tx_64_byte_packets", tx64octets_gb),
    FXGMAC_STAT("tx_65_to_127_byte_packets", tx65to127octets_gb),
    FXGMAC_STAT("tx_128_to_255_byte_packets", tx128to255octets_gb),
    FXGMAC_STAT("tx_256_to_511_byte_packets", tx256to511octets_gb),
    FXGMAC_STAT("tx_512_to_1023_byte_packets", tx512to1023octets_gb),
    FXGMAC_STAT("tx_1024_to_max_byte_packets", tx1024tomaxoctets_gb),
    FXGMAC_STAT("tx_underflow_errors", txunderflowerror),
    FXGMAC_STAT("tx_pause_frames", txpauseframes),
    FXGMAC_STAT("tx_single_collision", txsinglecollision_g),
    FXGMAC_STAT("tx_multiple_collision", txmultiplecollision_g),
    FXGMAC_STAT("tx_deferred_frames", txdeferredframes),
    FXGMAC_STAT("tx_late_collision_frames", txlatecollisionframes),
    FXGMAC_STAT("tx_excessive_collision_frames", txexcessivecollisionframes),
    FXGMAC_STAT("tx_carrier_error_frames", txcarriererrorframes),
    FXGMAC_STAT("tx_excessive_deferral_error", txexcessivedeferralerror),
    FXGMAC_STAT("tx_oversize_frames_good", txoversize_g),

    /* MMC RX counters */
    FXGMAC_STAT("rx_bytes", rxoctetcount_gb),
    FXGMAC_STAT("rx_bytes_good", rxoctetcount_g),
    FXGMAC_STAT("rx_packets", rxframecount_gb),
    FXGMAC_STAT("rx_unicast_packets_good", rxunicastframes_g),
    FXGMAC_STAT("rx_broadcast_packets_good", rxbroadcastframes_g),
    FXGMAC_STAT("rx_multicast_packets_good", rxmulticastframes_g),
    FXGMAC_STAT("rx_vlan_packets_mac", rxvlanframes_gb),
    FXGMAC_STAT("rx_64_byte_packets", rx64octets_gb),
    FXGMAC_STAT("rx_65_to_127_byte_packets", rx65to127octets_gb),
    FXGMAC_STAT("rx_128_to_255_byte_packets", rx128to255octets_gb),
    FXGMAC_STAT("rx_256_to_511_byte_packets", rx256to511octets_gb),
    FXGMAC_STAT("rx_512_to_1023_byte_packets", rx512to1023octets_gb),
    FXGMAC_STAT("rx_1024_to_max_byte_packets", rx1024tomaxoctets_gb),
    FXGMAC_STAT("rx_undersize_packets_good", rxundersize_g),
    FXGMAC_STAT("rx_oversize_packets_good", rxoversize_g),
    FXGMAC_STAT("rx_crc_errors", rxcrcerror),
    FXGMAC_STAT("rx_align_error", rxalignerror),
    FXGMAC_STAT("rx_crc_errors_small_packets", rxrunterror),
    FXGMAC_STAT("rx_crc_errors_giant_packets", rxjabbererror),
    FXGMAC_STAT("rx_length_errors", rxlengtherror),
    FXGMAC_STAT("rx_out_of_range_errors", rxoutofrangetype),
    FXGMAC_STAT("rx_fifo_overflow_errors", rxfifooverflow),
    FXGMAC_STAT("rx_watchdog_errors", rxwatchdogerror),
    FXGMAC_STAT("rx_pause_frames", rxpauseframes),
    FXGMAC_STAT("rx_receive_error_frames", rxreceiveerrorframe),
    FXGMAC_STAT("rx_control_frames_good", rxcontrolframe_g),

    /* Extra counters */
    FXGMAC_STAT("tx_tso_packets", tx_tso_packets),
    FXGMAC_STAT("rx_split_header_packets", rx_split_header_packets),
    FXGMAC_STAT("tx_process_stopped", tx_process_stopped),
    FXGMAC_STAT("rx_process_stopped", rx_process_stopped),
    FXGMAC_STAT("tx_buffer_unavailable", tx_buffer_unavailable),
    FXGMAC_STAT("rx_buffer_unavailable", rx_buffer_unavailable),
    FXGMAC_STAT("fatal_bus_error", fatal_bus_error),
    FXGMAC_STAT("tx_vlan_packets_net", tx_vlan_packets),
    FXGMAC_STAT("rx_vlan_packets_net", rx_vlan_packets),
    FXGMAC_STAT("napi_poll_isr", napi_poll_isr),
    FXGMAC_STAT("napi_poll_txtimer", napi_poll_txtimer),
    FXGMAC_STAT("alive_cnt_txtimer", cnt_alive_txtimer),

    FXGMAC_STAT("ephy_poll_timer", ephy_poll_timer_cnt),
    FXGMAC_STAT("mgmt_int_isr", mgmt_int_isr),
};

#define FXGMAC_STATS_COUNT	ARRAY_SIZE(fxgmac_gstring_stats)

static void fxgmac_ethtool_get_drvinfo(struct net_device *netdev,
				       struct ethtool_drvinfo *drvinfo)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    u32 ver = pdata->hw_feat.version;
    u32 sver, devid, userver;

    strscpy(drvinfo->driver, pdata->drv_name, sizeof(drvinfo->driver));
    strscpy(drvinfo->version, pdata->drv_ver, sizeof(drvinfo->version));
    strscpy(drvinfo->bus_info, dev_name(pdata->dev),
    sizeof(drvinfo->bus_info));
    /*
    * D|DEVID: Indicates the Device family
    * U|USERVER: User-defined Version
    */
    sver = FXGMAC_GET_REG_BITS(ver, MAC_VR_SVER_POS,
    		      MAC_VR_SVER_LEN);
    devid = FXGMAC_GET_REG_BITS(ver, MAC_VR_DEVID_POS,
    		    MAC_VR_DEVID_LEN);
    userver = FXGMAC_GET_REG_BITS(ver, MAC_VR_USERVER_POS,
    		      MAC_VR_USERVER_LEN);
    snprintf(drvinfo->fw_version, sizeof(drvinfo->fw_version),
     "S.D.U: %x.%x.%x", sver, devid, userver);
}

static u32 fxgmac_ethtool_get_msglevel(struct net_device *netdev)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    return pdata->msg_enable;
}

static void fxgmac_ethtool_set_msglevel(struct net_device *netdev,
					u32 msglevel)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    DPRINTK("fxmac, set msglvl from %08x to %08x\n", pdata->msg_enable, msglevel);
    pdata->msg_enable = msglevel;
}

static void fxgmac_ethtool_get_channels(struct net_device *netdev,
					struct ethtool_channels *channel)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
#if (FXGMAC_RSS_FEATURE_ENABLED)
    /* report maximum channels */
    channel->max_combined = FXGMAC_MAX_DMA_CHANNELS;
    channel->max_other = 0;
    channel->other_count = 0;

    /* record RSS queues */
    channel->combined_count = FXGMAC_MAX_DMA_CHANNELS;

    /* nothing else to report if RSS is disabled */
    if (channel->combined_count == 1)
    	return;
    DPRINTK("fxmac rss, get channels max=(combined %d,other %d),count(combined %d,other %d)\n", channel->max_combined, channel->max_other, channel->combined_count, channel->other_count);
#endif

    channel->max_rx = FXGMAC_MAX_DMA_CHANNELS;
    channel->max_tx = FXGMAC_MAX_DMA_CHANNELS;
    channel->rx_count = pdata->rx_q_count;
    channel->tx_count = pdata->tx_q_count;
    DPRINTK("fxmac, get channels max=(rx %d,tx %d),count(%d,%d)\n", channel->max_rx, channel->max_tx, channel->rx_count, channel->tx_count);
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0) ||\
       RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,1) )
static int fxgmac_ethtool_get_coalesce(struct net_device *netdev,
                                        struct ethtool_coalesce *ec,
                                        struct kernel_ethtool_coalesce *kernel_coal,
                                        struct netlink_ext_ack *extack)
#else
static int fxgmac_ethtool_get_coalesce(struct net_device *netdev, struct ethtool_coalesce *ec)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    memset(ec, 0, sizeof(struct ethtool_coalesce));
    ec->rx_coalesce_usecs = pdata->rx_usecs;
    ec->tx_coalesce_usecs = pdata->tx_usecs;
    /*If we need to assign values to other members,
    * we need to modify the supported_coalesce_params of fxgmac_ethtool_ops synchronously
    */
    //ec->rx_max_coalesced_frames = pdata->rx_frames;
    //ec->tx_max_coalesced_frames = pdata->tx_frames;

    DPRINTK("fxmac, get coalesce\n");
    return 0;
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,15,0) ||\
       RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,1) )
static int fxgmac_ethtool_set_coalesce(struct net_device *netdev,
                                        struct ethtool_coalesce *ec,
                                        struct kernel_ethtool_coalesce *kernel_coal,
                                        struct netlink_ext_ack *extack)
#else
static int fxgmac_ethtool_set_coalesce(struct net_device *netdev, struct ethtool_coalesce *ec)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    unsigned int rx_frames, rx_riwt, rx_usecs;
    unsigned int tx_frames;

    /* Check for not supported parameters */
    if ((ec->rx_coalesce_usecs_irq) || (ec->rx_max_coalesced_frames_irq) ||
        (ec->tx_coalesce_usecs_high) ||
        (ec->tx_max_coalesced_frames_irq) || (ec->tx_coalesce_usecs_irq) ||
        (ec->stats_block_coalesce_usecs) ||  (ec->pkt_rate_low) ||
        (ec->use_adaptive_rx_coalesce) || (ec->use_adaptive_tx_coalesce) ||
        (ec->rx_max_coalesced_frames_low) || (ec->rx_coalesce_usecs_low) ||
        (ec->tx_coalesce_usecs_low) || (ec->tx_max_coalesced_frames_low) ||
        (ec->pkt_rate_high) || (ec->rx_coalesce_usecs_high) ||
        (ec->rx_max_coalesced_frames_high) ||
        (ec->tx_max_coalesced_frames_high) ||
        (ec->rate_sample_interval))
    	return -EOPNOTSUPP;

    rx_usecs = ec->rx_coalesce_usecs;
    rx_riwt = hw_ops->usec_to_riwt(pdata, rx_usecs);
    rx_frames = ec->rx_max_coalesced_frames;
    tx_frames = ec->tx_max_coalesced_frames;

    if ((rx_riwt > FXGMAC_MAX_DMA_RIWT) ||
        (rx_riwt < FXGMAC_MIN_DMA_RIWT) ||
        (rx_frames > pdata->rx_desc_count))
    	return -EINVAL;

    if (tx_frames > pdata->tx_desc_count)
    	return -EINVAL;

    pdata->rx_riwt = rx_riwt;
    pdata->rx_usecs = rx_usecs;
    pdata->rx_frames = rx_frames;
    hw_ops->config_rx_coalesce(pdata);

    pdata->tx_frames = tx_frames;
    hw_ops->config_tx_coalesce(pdata);

    pdata->tx_usecs = ec->tx_coalesce_usecs;
    hw_ops->set_interrupt_moderation(pdata);

    DPRINTK("fxmac, set coalesce\n");
    return 0;
}

#if (FXGMAC_RSS_FEATURE_ENABLED)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
static u32 fxgmac_get_rxfh_key_size(struct net_device *netdev)
{
    return FXGMAC_RSS_HASH_KEY_SIZE;
}
#endif

static u32 fxgmac_rss_indir_size(struct net_device *netdev)
{
    return FXGMAC_RSS_MAX_TABLE_SIZE;
}

static void fxgmac_get_reta(struct fxgmac_pdata *pdata, u32 *indir)
{
    int i, reta_size = FXGMAC_RSS_MAX_TABLE_SIZE;
    u16 rss_m;
#ifdef FXGMAC_ONE_CHANNEL
    rss_m = FXGMAC_MAX_DMA_CHANNELS;
#else
    rss_m = FXGMAC_MAX_DMA_CHANNELS - 1; //mask for index of channel, 0-3
#endif

    for (i = 0; i < reta_size; i++)
    	indir[i] = pdata->rss_table[i] & rss_m;
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0) )
static int fxgmac_get_rxfh(struct net_device *netdev,struct ethtool_rxfh_param *rxfh)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    if (rxfh->hfunc)
    {
    	rxfh->hfunc = ETH_RSS_HASH_TOP;
    	DPRINTK("fxmac, get_rxfh for hash function\n");
    }

    if (rxfh->indir)
    {
    	fxgmac_get_reta(pdata, rxfh->indir);
    	DPRINTK("fxmac, get_rxfh for indirection tab\n");
    }

    if (rxfh->key)
    {
    	memcpy(rxfh->key, pdata->rss_key, fxgmac_get_rxfh_key_size(netdev));
    	DPRINTK("fxmac, get_rxfh  for hash key\n");
    }

    return 0;
}

static int fxgmac_set_rxfh(struct net_device *netdev, struct ethtool_rxfh_param *rxfh, struct netlink_ext_ack *ack)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int i;
    u32 reta_entries = fxgmac_rss_indir_size(netdev);
    int max_queues = FXGMAC_MAX_DMA_CHANNELS;

    DPRINTK("fxmac, set_rxfh callin, indir=%lx, key=%lx, func=%02x\n", (unsigned long)rxfh->indir, (unsigned long)rxfh->key, rxfh->hfunc);

    if (rxfh->hfunc)
    	return -EINVAL;

    /* Fill out the redirection table */
    if (rxfh->indir) {
#if FXGMAC_MSIX_CH0RXDIS_ENABLED
    	max_queues = max_queues; // kill warning
    	reta_entries = reta_entries;
    	i = i;
    	DPRINTK("fxmac, set_rxfh, change of indirect talbe is not supported.\n");
    	return -EINVAL;
#else
    	/* double check user input. */
    	for (i = 0; i < reta_entries; i++)
    	    if (rxfh->indir[i] >= max_queues)
    	        return -EINVAL;

    	for (i = 0; i < reta_entries; i++)
    	    pdata->rss_table[i] = rxfh->indir[i];

    	hw_ops->write_rss_lookup_table(pdata);
#endif
    }

    /* Fill out the rss hash key */
    if (FXGMAC_RSS_HASH_KEY_LINUX && rxfh->key) {
    	hw_ops->set_rss_hash_key(pdata, rxfh->key);
    }

    return 0;
}
#else
static int fxgmac_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
			  u8 *hfunc)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    if (hfunc)
    {
    	*hfunc = ETH_RSS_HASH_TOP;
    	DPRINTK("fxmac, get_rxfh for hash function\n");
    }

    if (indir)
    {
    	fxgmac_get_reta(pdata, indir);
    	DPRINTK("fxmac, get_rxfh for indirection tab\n");
    }

    if (key)
    {
    	memcpy(key, pdata->rss_key, fxgmac_get_rxfh_key_size(netdev));
    	DPRINTK("fxmac, get_rxfh  for hash key\n");
    }

    return 0;
}

static int fxgmac_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key, const u8 hfunc)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int i = 0;
    u32 reta_entries = fxgmac_rss_indir_size(netdev);
    int max_queues = FXGMAC_MAX_DMA_CHANNELS;

    DPRINTK("fxmac, set_rxfh callin, indir=%lx, key=%lx, func=%02x\n", (unsigned long)indir, (unsigned long)key, hfunc);

    if (hfunc)
    	return -EINVAL;

    /* Fill out the redirection table */
    if (indir) {
#if FXGMAC_MSIX_CH0RXDIS_ENABLED
    	max_queues = max_queues; // kill warning
    	reta_entries = reta_entries;
    	i = i;
    	DPRINTK("fxmac, set_rxfh, change of indirect talbe is not supported.\n");
    	return -EINVAL;
#else
    	/* double check user input. */
    	for (i = 0; i < reta_entries; i++)
    	    if (indir[i] >= max_queues)
    	        return -EINVAL;

    	for (i = 0; i < reta_entries; i++)
    	    pdata->rss_table[i] = indir[i];

    	hw_ops->write_rss_lookup_table(pdata);
#endif
    }

    /* Fill out the rss hash key */
    if (FXGMAC_RSS_HASH_KEY_LINUX && key) {
    	hw_ops->set_rss_hash_key(pdata, key);
    }

    return 0;
}
#endif

static int fxgmac_get_rss_hash_opts(struct fxgmac_pdata *pdata,
				   struct ethtool_rxnfc *cmd)
{
    u32 reg_opt;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    cmd->data = 0;

    reg_opt = hw_ops->get_rss_options(pdata);
    DPRINTK ("fxgmac_get_rss_hash_opts, hw=%02x, %02x\n", reg_opt, pdata->rss_options);

    if(reg_opt != pdata->rss_options)
    {
    	DPRINTK ("fxgmac_get_rss_hash_opts, warning, options are not consistent\n");
    }

    /* Report default options for RSS */
    switch (cmd->flow_type) {
    case TCP_V4_FLOW:
    case UDP_V4_FLOW:
    	if(((TCP_V4_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_TCP4TE_POS, MAC_RSSCR_TCP4TE_LEN))) ||
    		((UDP_V4_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_UDP4TE_POS, MAC_RSSCR_UDP4TE_LEN))))
    	{
    		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
    	}
        fallthrough;
    case SCTP_V4_FLOW:
    case AH_ESP_V4_FLOW:
    case AH_V4_FLOW:
    case ESP_V4_FLOW:
    case IPV4_FLOW:
    	if(((TCP_V4_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_TCP4TE_POS, MAC_RSSCR_TCP4TE_LEN))) ||
    		((UDP_V4_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_UDP4TE_POS, MAC_RSSCR_UDP4TE_LEN))) ||
    		(FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_IP4TE_POS, MAC_RSSCR_IP4TE_LEN)))
    	{
    		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
    	}
    	break;
    case TCP_V6_FLOW:
    case UDP_V6_FLOW:
    	if(((TCP_V6_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_TCP6TE_POS, MAC_RSSCR_TCP6TE_LEN))) ||
    		((UDP_V6_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_UDP6TE_POS, MAC_RSSCR_UDP6TE_LEN))))
    	{
    		cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
    	}
        fallthrough;
    case SCTP_V6_FLOW:
    case AH_ESP_V6_FLOW:
    case AH_V6_FLOW:
    case ESP_V6_FLOW:
    case IPV6_FLOW:
    	if(((TCP_V6_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_TCP6TE_POS, MAC_RSSCR_TCP6TE_LEN))) ||
    		((UDP_V6_FLOW == (cmd->flow_type)) && (FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_UDP6TE_POS, MAC_RSSCR_UDP6TE_LEN))) ||
    		(FXGMAC_GET_REG_BITS(pdata->rss_options, MAC_RSSCR_IP6TE_POS, MAC_RSSCR_IP6TE_LEN)))
    	{
    		cmd->data |= RXH_IP_SRC | RXH_IP_DST;
    	}
    	break;
    default:
    	return -EINVAL;
    }

    return 0;
}

static int fxgmac_get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd,
			   u32 *rule_locs)
{
    struct fxgmac_pdata *pdata = netdev_priv(dev);
    int ret = -EOPNOTSUPP;

    switch (cmd->cmd) {
    case ETHTOOL_GRXRINGS:
    	cmd->data = pdata->rx_q_count;
    	ret = 0;
    	DPRINTK("fxmac, get_rxnfc for rx ring cnt\n");
    	break;
    case ETHTOOL_GRXCLSRLCNT:
    	cmd->rule_cnt = 0;
    	ret = 0;
    	DPRINTK("fxmac, get_rxnfc for classify rule cnt\n");
    	break;
    case ETHTOOL_GRXCLSRULE:
    	DPRINTK("fxmac, get_rxnfc for classify rules\n");
    	ret = 0;//ixgbe_get_ethtool_fdir_entry(adapter, cmd);
    	break;
    case ETHTOOL_GRXCLSRLALL:
    	cmd->rule_cnt = 0;
    	ret = 0;
    	DPRINTK("fxmac, get_rxnfc for classify both cnt and rules\n");
    	break;
    case ETHTOOL_GRXFH:
    	ret = fxgmac_get_rss_hash_opts(pdata, cmd);
    	DPRINTK("fxmac, get_rxnfc for hash options\n");
    	break;
    default:
    	break;
    }

    return ret;
}

static int fxgmac_set_rss_hash_opt(struct fxgmac_pdata *pdata,
				  struct ethtool_rxnfc *nfc)
{
    u32 rssopt = 0; //pdata->rss_options;
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    DPRINTK("fxgmac_set_rss_hash_opt call in,nfc_data=%llx,cur opt=%x\n", nfc->data, pdata->rss_options);

    /*
    * For RSS, it does not support anything other than hashing
    * to queues on src,dst IPs and L4 ports
    */
    if (nfc->data & ~(RXH_IP_SRC | RXH_IP_DST |
    	  RXH_L4_B_0_1 | RXH_L4_B_2_3))
    return -EINVAL;

    switch (nfc->flow_type) {
        case TCP_V4_FLOW:
        case TCP_V6_FLOW:
            /* default to TCP flow and do nothting */
            if (!(nfc->data & RXH_IP_SRC) ||
                !(nfc->data & RXH_IP_DST) ||
                !(nfc->data & RXH_L4_B_0_1) ||
                !(nfc->data & RXH_L4_B_2_3))
            	return -EINVAL;
            if(TCP_V4_FLOW == (nfc->flow_type))
            {
            	rssopt = FXGMAC_SET_REG_BITS(
            		rssopt,
            		MAC_RSSCR_IP4TE_POS,
            		MAC_RSSCR_IP4TE_LEN, 1);
            	rssopt = FXGMAC_SET_REG_BITS(
            		rssopt,
            		MAC_RSSCR_TCP4TE_POS,
            		MAC_RSSCR_TCP4TE_LEN, 1);
            }

            if(TCP_V6_FLOW == (nfc->flow_type))
            {
            	rssopt = FXGMAC_SET_REG_BITS(
            		rssopt,
            		MAC_RSSCR_IP6TE_POS,
            		MAC_RSSCR_IP6TE_LEN, 1);
            	rssopt = FXGMAC_SET_REG_BITS(
            		rssopt,
            		MAC_RSSCR_TCP6TE_POS,
            		MAC_RSSCR_TCP6TE_LEN, 1);
            }
            break;

        case UDP_V4_FLOW:
            if (!(nfc->data & RXH_IP_SRC) ||
                !(nfc->data & RXH_IP_DST))
            	return -EINVAL;
            rssopt = FXGMAC_SET_REG_BITS(
            	rssopt,
            	MAC_RSSCR_IP4TE_POS,
            	MAC_RSSCR_IP4TE_LEN, 1);
            switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
                case 0:
                	break;
                case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
                	rssopt = FXGMAC_SET_REG_BITS(
                		rssopt,
                		MAC_RSSCR_UDP4TE_POS,
                		MAC_RSSCR_UDP4TE_LEN, 1);
                	break;
                default:
                	return -EINVAL;
                }
            break;
        case UDP_V6_FLOW:
            if (!(nfc->data & RXH_IP_SRC) ||
                !(nfc->data & RXH_IP_DST))
            	return -EINVAL;
            rssopt = FXGMAC_SET_REG_BITS(
            	rssopt,
            	MAC_RSSCR_IP6TE_POS,
            	MAC_RSSCR_IP6TE_LEN, 1);

            switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
                case 0:
                	break;
                case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
                	rssopt = FXGMAC_SET_REG_BITS(
                		rssopt,
                		MAC_RSSCR_UDP6TE_POS,
                		MAC_RSSCR_UDP6TE_LEN, 1);
                	break;
                default:
                	return -EINVAL;
            }
            break;
        case AH_ESP_V4_FLOW:
        case AH_V4_FLOW:
        case ESP_V4_FLOW:
        case SCTP_V4_FLOW:
        case AH_ESP_V6_FLOW:
        case AH_V6_FLOW:
        case ESP_V6_FLOW:
        case SCTP_V6_FLOW:
            if (!(nfc->data & RXH_IP_SRC) ||
                !(nfc->data & RXH_IP_DST) ||
                (nfc->data & RXH_L4_B_0_1) ||
                (nfc->data & RXH_L4_B_2_3))
            	return -EINVAL;
            break;
        default:
            return -EINVAL;
    }

    /* if options are changed, then update to hw */
    if (rssopt != pdata->rss_options) {
        if ((rssopt & UDP_RSS_FLAGS) &&
            !(pdata->rss_options & UDP_RSS_FLAGS))
        	DPRINTK("enabling UDP RSS: fragmented packets"
        	       " may arrive out of order to the stack above\n");

        DPRINTK("rss option changed from %x to %x\n", pdata->rss_options, rssopt);
        pdata->rss_options = rssopt;
        hw_ops->set_rss_options(pdata);
    }

    return 0;
}

static int fxgmac_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd)
{
    struct fxgmac_pdata *pdata = netdev_priv(dev);

    int ret = -EOPNOTSUPP;

    switch (cmd->cmd) {
    case ETHTOOL_SRXCLSRLINS:
    	DPRINTK("set_rxnfc for rx cls rule insert-n\\a\n");
    	break;
    case ETHTOOL_SRXCLSRLDEL:
    	DPRINTK("set_rxnfc for rx cls rule del-n\\a\n");
    	break;
    case ETHTOOL_SRXFH:
    	DPRINTK("set_rxnfc for rx rss option\n");
    	ret = fxgmac_set_rss_hash_opt(pdata, cmd);
    	break;
    default:
    	break;
    }

    return ret;
}
#endif //FXGMAC_RSS_FEATURE_ENABLED

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0) ||\
       RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,1) )
static void fxgmac_get_ringparam(struct net_device *netdev,
             struct ethtool_ringparam *ring,
             struct kernel_ethtool_ringparam *kernel_ring,
             struct netlink_ext_ack *exact)

#else
static void fxgmac_get_ringparam(struct net_device *netdev,
						struct ethtool_ringparam *ring)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    DPRINTK("fxmac, get_ringparam callin\n");

    ring->rx_max_pending = FXGMAC_RX_DESC_CNT;
    ring->tx_max_pending = FXGMAC_TX_DESC_CNT;
    ring->rx_mini_max_pending = 0;
    ring->rx_jumbo_max_pending = 0;
    ring->rx_pending = pdata->rx_desc_count;
    ring->tx_pending = pdata->tx_desc_count;
    ring->rx_mini_pending = 0;
    ring->rx_jumbo_pending = 0;
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(5,17,0) ||\
       RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(9,1) )
static int fxgmac_set_ringparam(struct net_device *netdev,
             struct ethtool_ringparam *ring,
             struct kernel_ethtool_ringparam *kernel_ring,
             struct netlink_ext_ack *exact)

#else
static int fxgmac_set_ringparam(struct net_device *netdev,
						struct ethtool_ringparam *ring)
#endif
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_desc_ops *desc_ops = &pdata->desc_ops;

    if (pdata->expansion.dev_state != FXGMAC_DEV_START)
        return 0;

    fxgmac_lock(pdata);
    DPRINTK("fxmac, set_ringparam callin\n");

    pdata->tx_desc_count = ring->tx_pending;
    pdata->rx_desc_count = ring->rx_pending;

    fxgmac_stop(pdata);
    fxgmac_free_tx_data(pdata);
    fxgmac_free_rx_data(pdata);
    desc_ops->alloc_channels_and_rings(pdata);
    fxgmac_start(pdata);
    fxgmac_unlock(pdata);

    return 0;
}

#if FXGMAC_WOL_FEATURE_ENABLED
static void fxgmac_get_wol(struct net_device *netdev,
			  struct ethtool_wolinfo *wol)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    wol->supported = WAKE_UCAST | WAKE_MCAST | WAKE_BCAST | WAKE_MAGIC | WAKE_ARP;
#if FXGMAC_WOL_UPON_EPHY_LINK
    wol->supported |= WAKE_PHY;
#endif

    wol->wolopts = 0;
    if (!(pdata->hw_feat.rwk) || !device_can_wakeup(/*pci_dev_to_dev*/(pdata->dev))) {
    	DPRINTK("fxgmac get_wol, pci does not support wakeup\n");
    	return;
    }
    wol->wolopts = pdata->expansion.wol;
}

// only supports four patterns, and patterns will be cleared on every call
static void fxgmac_set_pattern_data(struct fxgmac_pdata *pdata)
{
	struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
	u32                             ip_addr, i = 0;
    u8                              type_offset, op_offset, tip_offset;
    struct pattern_packet           packet;
    struct wol_bitmap_pattern       pattern[4]; // for WAKE_UCAST, WAKE_BCAST, WAKE_MCAST, WAKE_ARP.

    memset(pattern, 0, sizeof(struct wol_bitmap_pattern) * 4);

    //config ucast
    if (pdata->expansion.wol & WAKE_UCAST) {
        pattern[i].mask_info[0] = 0x3F;
        pattern[i].mask_size = sizeof(pattern[0].mask_info);
        memcpy(pattern[i].pattern_info, pdata->mac_addr, ETH_ALEN);
        pattern[i].pattern_offset = 0;
        i++;
    }

    // config bcast
    if (pdata->expansion.wol & WAKE_BCAST) {
        pattern[i].mask_info[0] = 0x3F;
        pattern[i].mask_size = sizeof(pattern[0].mask_info);
        memset(pattern[i].pattern_info, 0xFF, ETH_ALEN);
        pattern[i].pattern_offset = 0;
        i++;
    }

    // config mcast
    if (pdata->expansion.wol & WAKE_MCAST) {
        pattern[i].mask_info[0] = 0x7;
        pattern[i].mask_size = sizeof(pattern[0].mask_info);
        pattern[i].pattern_info[0] = 0x1;
        pattern[i].pattern_info[1] = 0x0;
        pattern[i].pattern_info[2] = 0x5E;
        pattern[i].pattern_offset = 0;
        i++;
    }

    // config arp
    if (pdata->expansion.wol & WAKE_ARP) {
        memset(pattern[i].mask_info, 0, sizeof(pattern[0].mask_info));
        type_offset = offsetof(struct pattern_packet, ar_pro);
        pattern[i].mask_info[type_offset / 8] |= 1 << type_offset % 8;
        type_offset++;
        pattern[i].mask_info[type_offset / 8] |= 1 << type_offset % 8;
        op_offset = offsetof(struct pattern_packet, ar_op);
        pattern[i].mask_info[op_offset / 8] |= 1 << op_offset % 8;
        op_offset++;
        pattern[i].mask_info[op_offset / 8] |= 1 << op_offset % 8;
        tip_offset = offsetof(struct pattern_packet, ar_tip);
        pattern[i].mask_info[tip_offset / 8] |= 1 << tip_offset % 8;
        tip_offset++;
        pattern[i].mask_info[tip_offset / 8] |= 1 << type_offset % 8;
        tip_offset++;
        pattern[i].mask_info[tip_offset / 8] |= 1 << type_offset % 8;
        tip_offset++;
        pattern[i].mask_info[tip_offset / 8] |= 1 << type_offset % 8;

        packet.ar_pro = 0x0 << 8 | 0x08; // arp type is 0x0800, notice that ar_pro and ar_op is big endian
        packet.ar_op = 0x1 << 8; // 1 is arp request,2 is arp replay, 3 is rarp request, 4 is rarp replay
        ip_addr = fxgmac_get_netdev_ip4addr(pdata);
        packet.ar_tip[0] = ip_addr & 0xFF;
        packet.ar_tip[1] = (ip_addr >> 8) & 0xFF;
        packet.ar_tip[2] = (ip_addr >> 16) & 0xFF;
        packet.ar_tip[3] = (ip_addr >> 24) & 0xFF;
        memcpy(pattern[i].pattern_info, &packet, MAX_PATTERN_SIZE);
        pattern[i].mask_size = sizeof(pattern[0].mask_info);
        pattern[i].pattern_offset = 0;
        i++;
    }

	hw_ops->set_wake_pattern(pdata, pattern, i);
}

void fxgmac_config_wol(struct fxgmac_pdata *pdata, int en)
{
    /* enable or disable WOL. this function only set wake-up type, and power related configure
     * will be in other place, see power management.
     */
	struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    if (!pdata->hw_feat.rwk) {
        netdev_err(pdata->netdev, "error configuring WOL - not supported.\n");
        return;
    }

	hw_ops->disable_wake_magic_pattern(pdata);
	hw_ops->disable_wake_pattern(pdata);
	hw_ops->disable_wake_link_change(pdata);

    if(en) {
        /* config mac address for rx of magic or ucast */
		hw_ops->set_mac_address(pdata, (u8*)(pdata->netdev->dev_addr));

        /* Enable Magic packet */
        if (pdata->expansion.wol & WAKE_MAGIC) {
			hw_ops->enable_wake_magic_pattern(pdata);
        }

        /* Enable global unicast packet */
        if (pdata->expansion.wol & WAKE_UCAST
            || pdata->expansion.wol & WAKE_MCAST
            || pdata->expansion.wol & WAKE_BCAST
            || pdata->expansion.wol & WAKE_ARP) {
            hw_ops->enable_wake_pattern(pdata);
        }

        /* Enable ephy link change */
        if ((FXGMAC_WOL_UPON_EPHY_LINK) && (pdata->expansion.wol & WAKE_PHY)) {
			hw_ops->enable_wake_link_change(pdata);
        }
    }
    device_set_wakeup_enable(/*pci_dev_to_dev*/(pdata->dev), en);

    DPRINTK("config_wol callout\n");
}

static int fxgmac_set_wol(struct net_device *netdev, struct ethtool_wolinfo *wol)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    //currently, we do not support these options
#if FXGMAC_WOL_UPON_EPHY_LINK
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    if (wol->wolopts & (WAKE_MAGICSECURE | WAKE_FILTER)) {
#else
    if (wol->wolopts & WAKE_MAGICSECURE) {
#endif
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0))
    if (wol->wolopts & (WAKE_PHY | WAKE_MAGICSECURE | WAKE_FILTER)) {
#else
    if (wol->wolopts & (WAKE_PHY | WAKE_MAGICSECURE)) {
#endif
#endif
    	DPRINTK("fxmac, set_wol, not supported wol options, 0x%x\n", wol->wolopts);
    	return -EOPNOTSUPP;
    }

    if (!(pdata->hw_feat.rwk)) {
    	DPRINTK("fxmac, set_wol, hw wol feature is n/a\n");
    	return (wol->wolopts ? -EOPNOTSUPP : 0);
    }

    pdata->expansion.wol = 0;
    if (wol->wolopts & WAKE_UCAST)
            pdata->expansion.wol |= WAKE_UCAST;

    if (wol->wolopts & WAKE_MCAST)
            pdata->expansion.wol |= WAKE_MCAST;

    if (wol->wolopts & WAKE_BCAST)
            pdata->expansion.wol |= WAKE_BCAST;

    if (wol->wolopts & WAKE_MAGIC)
            pdata->expansion.wol |= WAKE_MAGIC;

    if (wol->wolopts & WAKE_PHY)
            pdata->expansion.wol |= WAKE_PHY;

    if (wol->wolopts & WAKE_ARP)
            pdata->expansion.wol |= WAKE_ARP;

	fxgmac_set_pattern_data(pdata);

	fxgmac_config_wol(pdata, (!!(pdata->expansion.wol)));
    DPRINTK("fxmac, set_wol, opt=0x%x, 0x%x\n", wol->wolopts, pdata->expansion.wol);

    return 0;
}
#endif /*FXGMAC_WOL_FEATURE_ENABLED*/

static int fxgmac_get_regs_len(struct net_device __always_unused *netdev)
{
    return FXGMAC_EPHY_REGS_LEN * sizeof(u32);
}

static void fxgmac_get_regs(struct net_device *netdev, struct ethtool_regs *regs,
			   void *p)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;

    u32 *regs_buff = p;
    u8 i;

    memset(p, 0, FXGMAC_EPHY_REGS_LEN * sizeof(u32));
    for (i = REG_MII_BMCR; i < FXGMAC_EPHY_REGS_LEN; i++) {
    	hw_ops->read_ephy_reg(pdata, i, (unsigned int *)&regs_buff[i]);
    }
    regs->version = regs_buff[REG_MII_PHYSID1] << 16 | regs_buff[REG_MII_PHYSID2];
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0))
static int fxgmac_get_link_ksettings(struct net_device *netdev,
				    struct ethtool_link_ksettings *cmd)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    u32 duplex, regval, link_status;
    u32 adv = 0xFFFFFFFF;
    int ret;
#if 0
    ret = fxgmac_ephy_autoneg_ability_get(pdata, &adv);
    if (ret < 0)
        return -ETIMEDOUT;
#endif

    ethtool_link_ksettings_zero_link_mode(cmd, supported);
    ethtool_link_ksettings_zero_link_mode(cmd, advertising);

    /* set the supported link speeds */
    ethtool_link_ksettings_add_link_mode(cmd, supported, 1000baseT_Full);
    ethtool_link_ksettings_add_link_mode(cmd, supported, 100baseT_Full);
    ethtool_link_ksettings_add_link_mode(cmd, supported, 100baseT_Half);
    ethtool_link_ksettings_add_link_mode(cmd, supported, 10baseT_Full);
    ethtool_link_ksettings_add_link_mode(cmd, supported, 10baseT_Half);

    /* Indicate pause support */
    ethtool_link_ksettings_add_link_mode(cmd, supported, Pause);
    ethtool_link_ksettings_add_link_mode(cmd, supported, Asym_Pause);
    ret = hw_ops->read_ephy_reg(pdata, REG_MII_ADVERTISE, &regval);
    if (ret < 0)
        return ret;

    if (FXGMAC_GET_REG_BITS(regval, PHY_MII_ADVERTISE_PAUSE_POS, PHY_MII_ADVERTISE_PAUSE_LEN))
        ethtool_link_ksettings_add_link_mode(cmd, advertising, Pause);
    if (FXGMAC_GET_REG_BITS(regval, PHY_MII_ADVERTISE_ASYPAUSE_POS, PHY_MII_ADVERTISE_ASYPAUSE_LEN))
        ethtool_link_ksettings_add_link_mode(cmd, advertising, Asym_Pause);

    ethtool_link_ksettings_add_link_mode(cmd, supported, MII);
    cmd->base.port = PORT_MII;

    ethtool_link_ksettings_add_link_mode(cmd, supported, Autoneg);
    ret = hw_ops->read_ephy_reg(pdata, REG_MII_BMCR, &regval);
    if (ret < 0)
        return ret;

    regval = FXGMAC_GET_REG_BITS(regval, PHY_CR_AUTOENG_POS, PHY_CR_AUTOENG_LEN);
    if (regval) {
        if (pdata->phy_autoeng)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, Autoneg);
        else
            goto FORCE_MODE;
            //clear_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, cmd->link_modes.advertising);

        ret = hw_ops->read_ephy_reg(pdata, REG_MII_ADVERTISE, &adv);
        if (ret < 0)
                return ret;

        if (adv & FXGMAC_ADVERTISE_10HALF)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, 10baseT_Half);
        if (adv & FXGMAC_ADVERTISE_10FULL)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, 10baseT_Full);
        if (adv & FXGMAC_ADVERTISE_100HALF)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, 100baseT_Half);
        if (adv & FXGMAC_ADVERTISE_100FULL)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, 100baseT_Full);

        ret = hw_ops->read_ephy_reg(pdata, REG_MII_CTRL1000, &adv);
        if (ret < 0)
                return ret;

        if (adv & FXGMAC_ADVERTISE_1000FULL)
            ethtool_link_ksettings_add_link_mode(cmd, advertising, 1000baseT_Full);
    }
    else {
FORCE_MODE:
        clear_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, cmd->link_modes.advertising);
        switch (pdata->phy_speed) {
            case SPEED_1000M:
                if (pdata->phy_duplex)
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 1000baseT_Full);
                else
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 1000baseT_Half);
                break;
            case SPEED_100M:
                if (pdata->phy_duplex)
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 100baseT_Full);
                else
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 100baseT_Half);
                break;
            case SPEED_10M:
                if (pdata->phy_duplex)
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 10baseT_Full);
                else
                    ethtool_link_ksettings_add_link_mode(cmd, advertising, 10baseT_Half);
                break;
            default:
                break;
        }
    }
    cmd->base.autoneg = pdata->phy_autoeng ? regval : 0;

    regval = 0;
    ret = hw_ops->read_ephy_reg(pdata, REG_MII_SPEC_STATUS, &regval);
    if (ret < 0)
	return ret;

    link_status = regval & (BIT(FXGMAC_EPHY_LINK_STATUS_BIT));
    if (link_status) {
        duplex = FXGMAC_GET_REG_BITS(regval, PHY_MII_SPEC_DUPLEX_POS, PHY_MII_SPEC_DUPLEX_LEN);
        cmd->base.duplex = duplex;
        cmd->base.speed = pdata->phy_speed;
    }else {
        cmd->base.duplex = DUPLEX_UNKNOWN;
        cmd->base.speed = SPEED_UNKNOWN;
    }

    return 0;
}

static int fxgmac_set_link_ksettings(struct net_device *netdev,
				    const struct ethtool_link_ksettings *cmd)
{
    u32 advertising, support;
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    int ret;

    if (cmd->base.speed == SPEED_1000 && cmd->base.duplex == DUPLEX_HALF)
        return -EINVAL;

    pdata->phy_autoeng = cmd->base.autoneg;
    pdata->phy_duplex = cmd->base.duplex;
    pdata->phy_speed = cmd->base.speed;

    ethtool_convert_link_mode_to_legacy_u32(&advertising, cmd->link_modes.advertising);
    ethtool_convert_link_mode_to_legacy_u32(&support, cmd->link_modes.supported);
    advertising &= support;

    if (pdata->phy_autoeng || (!pdata->phy_autoeng && cmd->base.speed == SPEED_1000)){
        pdata->expansion.phy_link = false;
        //pdata->phy_autoeng = AUTONEG_ENABLE;
        ret = hw_ops->phy_config(pdata);
        if (ret < 0)
           return ret;
    } else {
        fxgmac_phy_force_mode(pdata);
    }

    /* Save speed is used to restore it when resuming */
    pdata->expansion.pre_phy_speed = cmd->base.speed;
    pdata->expansion.pre_phy_autoneg = cmd->base.autoneg;
    pdata->expansion.pre_phy_duplex = cmd->base.duplex;

    return 0;
}
#endif

#if FXGMAC_PAUSE_FEATURE_ENABLED
static void fxgmac_get_pauseparam(struct net_device *netdev,
				 struct ethtool_pauseparam *pause)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);

    pause->autoneg = pdata->phy_autoeng;
    pause->rx_pause = pdata->rx_pause;
    pause->tx_pause = pdata->tx_pause;

    DPRINTK("fxmac get_pauseparam done, rx=%d, tx=%d\n", pdata->rx_pause, pdata->tx_pause);
}

static int fxgmac_set_pauseparam(struct net_device *netdev,
				struct ethtool_pauseparam *pause)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    unsigned int pre_rx_pause = pdata->rx_pause;
    unsigned int pre_tx_pause = pdata->tx_pause;
    u32 adv;
    int ret;
    int enable_pause = 0;

    pdata->rx_pause = pause->rx_pause;
    pdata->tx_pause = pause->tx_pause;

    if (pdata->rx_pause || pdata->tx_pause)
        enable_pause = 1;

    if(pre_rx_pause != pdata->rx_pause) {
    	hw_ops->config_rx_flow_control(pdata);
    	DPRINTK("fxgmac set pause parameter, rx from %d to %d\n", pre_rx_pause, pdata->rx_pause);
    }
    if(pre_tx_pause != pdata->tx_pause) {
    	hw_ops->config_tx_flow_control(pdata);
    	DPRINTK("fxgmac set pause parameter, tx from %d to %d\n", pre_tx_pause, pdata->tx_pause);
    }

    if (pause->autoneg) {
        ret = hw_ops->read_ephy_reg(pdata, REG_MII_ADVERTISE, &adv);
        if (ret < 0)
            return ret;
        adv = FXGMAC_SET_REG_BITS(adv, PHY_MII_ADVERTISE_PAUSE_POS,
                                        PHY_MII_ADVERTISE_PAUSE_LEN,
                                        enable_pause);
        adv = FXGMAC_SET_REG_BITS(adv, PHY_MII_ADVERTISE_ASYPAUSE_POS,
                                        PHY_MII_ADVERTISE_ASYPAUSE_LEN,
                                        enable_pause);
        ret = hw_ops->write_ephy_reg(pdata, REG_MII_ADVERTISE, adv);
        if (ret < 0) {
            return ret;
        }

        ret = hw_ops->read_ephy_reg(pdata, REG_MII_BMCR, &adv);
        if (ret < 0)
            return ret;
        adv = FXGMAC_SET_REG_BITS(adv, PHY_CR_RE_AUTOENG_POS, PHY_CR_RE_AUTOENG_LEN, 1);
        ret = hw_ops->write_ephy_reg(pdata, REG_MII_BMCR, adv);
        if (ret < 0)
            return ret;
    }else {
        DPRINTK("Can't set phy pause because autoneg is off.\n");
    }

    DPRINTK("fxgmac set pause parameter, autoneg=%d, rx=%d, tx=%d\n", pause->autoneg, pause->rx_pause, pause->tx_pause);

    return 0;
}
#endif /*FXGMAC_PAUSE_FEATURE_ENABLED*/

static void fxgmac_ethtool_get_strings(struct net_device *netdev,
				       u32 stringset, u8 *data)
{
    int i;

    switch (stringset) {
    case ETH_SS_STATS:
    	for (i = 0; i < FXGMAC_STATS_COUNT; i++) {
    		memcpy(data, fxgmac_gstring_stats[i].stat_string,
    		       strlen(fxgmac_gstring_stats[i].stat_string));
    		data += ETH_GSTRING_LEN;
    	}
    	break;
    default:
    	WARN_ON(1);
    	break;
    }
}

static int fxgmac_ethtool_get_sset_count(struct net_device *netdev,
					 int stringset)
{
    int ret;

    switch (stringset) {
    case ETH_SS_STATS:
    	ret = FXGMAC_STATS_COUNT;
    	break;

    default:
    	ret = -EOPNOTSUPP;
    }

    return ret;
}

static void fxgmac_ethtool_get_ethtool_stats(struct net_device *netdev,
					     struct ethtool_stats *stats,
					     u64 *data)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    u8 *stat;
    int i;

#if FXGMAC_PM_FEATURE_ENABLED
    if(!test_bit(FXGMAC_POWER_STATE_DOWN, &pdata->expansion.powerstate))
#endif
    {
    	pdata->hw_ops.read_mmc_stats(pdata);
    }

    for (i = 0; i < FXGMAC_STATS_COUNT; i++) {
    	stat = (u8 *)pdata + fxgmac_gstring_stats[i].stat_offset;
    	*data++ = *(u64 *)stat;
    }
}

#if 0
static inline bool fxgmac_removed(void __iomem *addr)
{
    return unlikely(!addr);
}
#endif

static int fxgmac_ethtool_reset(struct net_device *netdev, u32 *flag)
{
    struct fxgmac_pdata *pdata = netdev_priv(netdev);
    struct fxgmac_hw_ops *hw_ops = &pdata->hw_ops;
    u32 val;
    int ret = 0;

    val = (*flag & ETH_RESET_ALL) || (*flag & ETH_RESET_PHY);
    if (!val) {
        DPRINTK("Operation not support.\n");
        return -EINVAL;
    }

    switch(*flag) {
        case ETH_RESET_ALL:
            fxgmac_restart_dev(pdata);
            *flag = 0;
            break;
        case ETH_RESET_PHY:
            /*
             * power off and on the phy in order to properly
             * configure the MAC timing
             */
            hw_ops->read_ephy_reg(pdata, REG_MII_BMCR, &val);
            val = FXGMAC_SET_REG_BITS(val, PHY_CR_POWER_POS,
                                                PHY_CR_POWER_LEN,
                                                    PHY_POWER_DOWN);
            ret = hw_ops->write_ephy_reg(pdata, REG_MII_BMCR, val);
            if(ret < 0)
                return ret;

            usleep_range_ex(pdata->pAdapter, 9000, 10000);
            val = FXGMAC_SET_REG_BITS(val, PHY_CR_POWER_POS,
                                                PHY_CR_POWER_LEN,
                                                    PHY_POWER_UP);
            ret = hw_ops->write_ephy_reg(pdata, REG_MII_BMCR, val);
            if(ret < 0)
                return ret;

            *flag = 0;
            break;
        default:
            break;
    }

    return 0;
}

static const struct ethtool_ops fxgmac_ethtool_ops = {
    .get_drvinfo = fxgmac_ethtool_get_drvinfo,
    .get_link = ethtool_op_get_link,
    .get_msglevel = fxgmac_ethtool_get_msglevel,
    .set_msglevel = fxgmac_ethtool_set_msglevel,
    .get_channels = fxgmac_ethtool_get_channels,
    .get_coalesce = fxgmac_ethtool_get_coalesce,
    .set_coalesce = fxgmac_ethtool_set_coalesce,
    .reset        = fxgmac_ethtool_reset,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
    /*
    * The process of set is to get first and then set,
    * and the result of get is preserved for values that have not been modified.
    *
    * Therefore, when using, it is necessary to ensure that this macro and the
    * assignment operation in the get_coalesce are one-to-one correspondence,
    * otherwise the macro and parameters will be verified when set, and the error
    * of "Operation not supported " will be reported if the verification fails
    */
#ifdef ETHTOOL_COALESCE_USECS
    .supported_coalesce_params = ETHTOOL_COALESCE_USECS,
#endif
#endif
    .get_strings = fxgmac_ethtool_get_strings,
    .get_sset_count = fxgmac_ethtool_get_sset_count,
    .get_ethtool_stats = fxgmac_ethtool_get_ethtool_stats,
    .get_regs_len	= fxgmac_get_regs_len,
    .get_regs		= fxgmac_get_regs,
    .get_ringparam	= fxgmac_get_ringparam,
    .set_ringparam	= fxgmac_set_ringparam,
#if (FXGMAC_RSS_FEATURE_ENABLED)
    .get_rxnfc		= fxgmac_get_rxnfc,
    .set_rxnfc		= fxgmac_set_rxnfc,
    .get_rxfh_indir_size	= fxgmac_rss_indir_size,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0))
    .get_rxfh_key_size	= fxgmac_get_rxfh_key_size,
#endif
    .get_rxfh		= fxgmac_get_rxfh,
    .set_rxfh		= fxgmac_set_rxfh,
#endif
#if (FXGMAC_WOL_FEATURE_ENABLED)
    .get_wol		= fxgmac_get_wol,
    .set_wol		= fxgmac_set_wol,
#endif
#if (FXGMAC_PAUSE_FEATURE_ENABLED)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0))
    .get_link_ksettings = fxgmac_get_link_ksettings,
    .set_link_ksettings = fxgmac_set_link_ksettings,
#endif
    .get_pauseparam = fxgmac_get_pauseparam,
    .set_pauseparam = fxgmac_set_pauseparam,
#endif
};

const struct ethtool_ops *fxgmac_get_ethtool_ops(void)
{
    return &fxgmac_ethtool_ops;
}
