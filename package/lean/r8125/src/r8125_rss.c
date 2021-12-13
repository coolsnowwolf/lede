// SPDX-License-Identifier: GPL-2.0-only
/*
################################################################################
#
# r8168 is the Linux device driver released for Realtek Gigabit Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2021 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

#include <linux/version.h>
#include "r8125.h"

enum rtl8125_rss_register_content {
        /* RSS */
        RSS_CTRL_TCP_IPV4_SUPP = (1 << 0),
        RSS_CTRL_IPV4_SUPP  = (1 << 1),
        RSS_CTRL_TCP_IPV6_SUPP  = (1 << 2),
        RSS_CTRL_IPV6_SUPP  = (1 << 3),
        RSS_HALF_SUPP  = (1 << 7),
        RSS_CTRL_UDP_IPV4_SUPP  = (1 << 11),
        RSS_CTRL_UDP_IPV6_SUPP  = (1 << 12),
        RSS_QUAD_CPU_EN  = (1 << 16),
        RSS_HQ_Q_SUP_R  = (1 << 31),
};

static int rtl8125_get_rss_hash_opts(struct rtl8125_private *tp,
                                     struct ethtool_rxnfc *cmd)
{
        cmd->data = 0;

        /* Report default options for RSS */
        switch (cmd->flow_type) {
        case TCP_V4_FLOW:
                cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
        /* fallthrough */
        case UDP_V4_FLOW:
                if (tp->rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV4)
                        cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
        /* fallthrough */
        case IPV4_FLOW:
                cmd->data |= RXH_IP_SRC | RXH_IP_DST;
                break;
        case TCP_V6_FLOW:
                cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
        /* fallthrough */
        case UDP_V6_FLOW:
                if (tp->rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV6)
                        cmd->data |= RXH_L4_B_0_1 | RXH_L4_B_2_3;
        /* fallthrough */
        case IPV6_FLOW:
                cmd->data |= RXH_IP_SRC | RXH_IP_DST;
                break;
        default:
                return -EINVAL;
        }

        return 0;
}

int rtl8125_get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd,
                      u32 *rule_locs)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret = -EOPNOTSUPP;

        netif_info(tp, drv, tp->dev, "rss get rxnfc\n");

        if (!(dev->features & NETIF_F_RXHASH))
                return ret;

        switch (cmd->cmd) {
        case ETHTOOL_GRXRINGS:
                cmd->data = rtl8125_tot_rx_rings(tp);
                ret = 0;
                break;
        case ETHTOOL_GRXFH:
                ret = rtl8125_get_rss_hash_opts(tp, cmd);
                break;
        default:
                break;
        }

        return ret;
}

u32 rtl8125_rss_indir_tbl_entries(struct rtl8125_private *tp)
{
        return tp->HwSuppIndirTblEntries;
}

#define RSS_MASK_BITS_OFFSET (8)
#define RSS_CPU_NUM_OFFSET (16)
#define RTL8125_UDP_RSS_FLAGS (RTL_8125_RSS_FLAG_HASH_UDP_IPV4 | \
		       RTL_8125_RSS_FLAG_HASH_UDP_IPV6)
static int _rtl8125_set_rss_hash_opt(struct rtl8125_private *tp)
{
        u32 rss_flags = tp->rss_flags;
        u32 hash_mask_len;
        u32 rss_ctrl;

        rss_ctrl = ilog2(rtl8125_tot_rx_rings(tp));
        rss_ctrl &= (BIT_0 | BIT_1 | BIT_2);
        rss_ctrl <<= RSS_CPU_NUM_OFFSET;

        /* Perform hash on these packet types */
        rss_ctrl |= RSS_CTRL_TCP_IPV4_SUPP
                    | RSS_CTRL_IPV4_SUPP
                    | RSS_CTRL_IPV6_SUPP
                    | RSS_CTRL_TCP_IPV6_SUPP;

        if (rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV4)
                rss_ctrl |= RSS_CTRL_UDP_IPV4_SUPP;

        if (rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV6)
                rss_ctrl |= RSS_CTRL_UDP_IPV6_SUPP;

        hash_mask_len = ilog2(rtl8125_rss_indir_tbl_entries(tp));
        hash_mask_len &= (BIT_0 | BIT_1 | BIT_2);
        rss_ctrl |= hash_mask_len << RSS_MASK_BITS_OFFSET;

        RTL_W32(tp, RSS_CTRL_8125, rss_ctrl);

        return 0;
}

static int rtl8125_set_rss_hash_opt(struct rtl8125_private *tp,
                                    struct ethtool_rxnfc *nfc)
{
        u32 rss_flags = tp->rss_flags;

        netif_info(tp, drv, tp->dev, "rss set hash\n");

        /*
         * RSS does not support anything other than hashing
         * to queues on src and dst IPs and ports
         */
        if (nfc->data & ~(RXH_IP_SRC | RXH_IP_DST |
                          RXH_L4_B_0_1 | RXH_L4_B_2_3))
                return -EINVAL;

        switch (nfc->flow_type) {
        case TCP_V4_FLOW:
        case TCP_V6_FLOW:
                if (!(nfc->data & RXH_IP_SRC) ||
                    !(nfc->data & RXH_IP_DST) ||
                    !(nfc->data & RXH_L4_B_0_1) ||
                    !(nfc->data & RXH_L4_B_2_3))
                        return -EINVAL;
                break;
        case UDP_V4_FLOW:
                if (!(nfc->data & RXH_IP_SRC) ||
                    !(nfc->data & RXH_IP_DST))
                        return -EINVAL;
                switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
                case 0:
                        rss_flags &= ~RTL_8125_RSS_FLAG_HASH_UDP_IPV4;
                        break;
                case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
                        rss_flags |= RTL_8125_RSS_FLAG_HASH_UDP_IPV4;
                        break;
                default:
                        return -EINVAL;
                }
                break;
        case UDP_V6_FLOW:
                if (!(nfc->data & RXH_IP_SRC) ||
                    !(nfc->data & RXH_IP_DST))
                        return -EINVAL;
                switch (nfc->data & (RXH_L4_B_0_1 | RXH_L4_B_2_3)) {
                case 0:
                        rss_flags &= ~RTL_8125_RSS_FLAG_HASH_UDP_IPV6;
                        break;
                case (RXH_L4_B_0_1 | RXH_L4_B_2_3):
                        rss_flags |= RTL_8125_RSS_FLAG_HASH_UDP_IPV6;
                        break;
                default:
                        return -EINVAL;
                }
                break;
        case SCTP_V4_FLOW:
        case AH_ESP_V4_FLOW:
        case AH_V4_FLOW:
        case ESP_V4_FLOW:
        case SCTP_V6_FLOW:
        case AH_ESP_V6_FLOW:
        case AH_V6_FLOW:
        case ESP_V6_FLOW:
        case IP_USER_FLOW:
        case ETHER_FLOW:
                /* RSS is not supported for these protocols */
                if (nfc->data) {
                        netif_err(tp, drv, tp->dev, "Command parameters not supported\n");
                        return -EINVAL;
                }
                return 0;
                break;
        default:
                return -EINVAL;
        }

        /* if we changed something we need to update flags */
        if (rss_flags != tp->rss_flags) {
                u32 rss_ctrl = RTL_R32(tp, RSS_CTRL_8125);

                if ((rss_flags & RTL8125_UDP_RSS_FLAGS) &&
                    !(tp->rss_flags & RTL8125_UDP_RSS_FLAGS))
                        netdev_warn(tp->dev,
                                    "enabling UDP RSS: fragmented packets may "
                                    "arrive out of order to the stack above\n");

                tp->rss_flags = rss_flags;

                /* Perform hash on these packet types */
                rss_ctrl |= RSS_CTRL_TCP_IPV4_SUPP
                            | RSS_CTRL_IPV4_SUPP
                            | RSS_CTRL_IPV6_SUPP
                            | RSS_CTRL_TCP_IPV6_SUPP;

                rss_ctrl &= ~(RSS_CTRL_UDP_IPV4_SUPP |
                              RSS_CTRL_UDP_IPV6_SUPP);

                if (rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV4)
                        rss_ctrl |= RSS_CTRL_UDP_IPV4_SUPP;

                if (rss_flags & RTL_8125_RSS_FLAG_HASH_UDP_IPV6)
                        rss_ctrl |= RSS_CTRL_UDP_IPV6_SUPP;

                RTL_W32(tp, RSS_CTRL_8125, rss_ctrl);
        }

        return 0;
}

int rtl8125_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int ret = -EOPNOTSUPP;

        netif_info(tp, drv, tp->dev, "rss set rxnfc\n");

        if (!(dev->features & NETIF_F_RXHASH))
                return ret;

        switch (cmd->cmd) {
        case ETHTOOL_SRXFH:
                ret = rtl8125_set_rss_hash_opt(tp, cmd);
                break;
        default:
                break;
        }

        return ret;
}

static u32 _rtl8125_get_rxfh_key_size(struct rtl8125_private *tp)
{
        return sizeof(tp->rss_key);
}

u32 rtl8125_get_rxfh_key_size(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        netif_info(tp, drv, tp->dev, "rss get key size\n");

        if (!(dev->features & NETIF_F_RXHASH))
                return 0;

        return _rtl8125_get_rxfh_key_size(tp);
}

u32 rtl8125_rss_indir_size(struct net_device *dev)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        netif_info(tp, drv, tp->dev, "rss get indir tbl size\n");

        if (!(dev->features & NETIF_F_RXHASH))
                return 0;

        return rtl8125_rss_indir_tbl_entries(tp);
}

static void rtl8125_get_reta(struct rtl8125_private *tp, u32 *indir)
{
        int i, reta_size = rtl8125_rss_indir_tbl_entries(tp);

        for (i = 0; i < reta_size; i++)
                indir[i] = tp->rss_indir_tbl[i];
}

int rtl8125_get_rxfh(struct net_device *dev, u32 *indir, u8 *key,
                     u8 *hfunc)
{
        struct rtl8125_private *tp = netdev_priv(dev);

        netif_info(tp, drv, tp->dev, "rss get rxfh\n");

        if (!(dev->features & NETIF_F_RXHASH))
                return -EOPNOTSUPP;

        if (hfunc)
                *hfunc = ETH_RSS_HASH_TOP;

        if (indir)
                rtl8125_get_reta(tp, indir);

        if (key)
                memcpy(key, tp->rss_key, rtl8125_get_rxfh_key_size(dev));

        return 0;
}

static u32 rtl8125_rss_key_reg(struct rtl8125_private *tp)
{
        return RSS_KEY_8125;
}

static u32 rtl8125_rss_indir_tbl_reg(struct rtl8125_private *tp)
{
        return RSS_INDIRECTION_TBL_8125_V2;
}

static void rtl8125_store_reta(struct rtl8125_private *tp)
{
        u16 indir_tbl_reg = rtl8125_rss_indir_tbl_reg(tp);
        u32 i, reta_entries = rtl8125_rss_indir_tbl_entries(tp);
        u32 reta = 0;
        u8 *indir_tbl = tp->rss_indir_tbl;

        /* Write redirection table to HW */
        for (i = 0; i < reta_entries; i++) {
                reta |= indir_tbl[i] << (i & 0x3) * 8;
                if ((i & 3) == 3) {
                        RTL_W32(tp, indir_tbl_reg, reta);

                        indir_tbl_reg += 4;
                        reta = 0;
                }
        }
}

static void rtl8125_store_rss_key(struct rtl8125_private *tp)
{
        const u16 rss_key_reg = rtl8125_rss_key_reg(tp);
        u32 i, rss_key_size = _rtl8125_get_rxfh_key_size(tp);
        u32 *rss_key = (u32*)tp->rss_key;

        /* Write redirection table to HW */
        for (i = 0; i < rss_key_size; i+=4)
                RTL_W32(tp, rss_key_reg + i, *rss_key++);
}

int rtl8125_set_rxfh(struct net_device *dev, const u32 *indir,
                     const u8 *key, const u8 hfunc)
{
        struct rtl8125_private *tp = netdev_priv(dev);
        int i;
        u32 reta_entries = rtl8125_rss_indir_tbl_entries(tp);

        netif_info(tp, drv, tp->dev, "rss set rxfh\n");

        /* We require at least one supported parameter to be changed and no
         * change in any of the unsupported parameters
         */
        if (hfunc != ETH_RSS_HASH_NO_CHANGE && hfunc != ETH_RSS_HASH_TOP)
                return -EOPNOTSUPP;

        /* Fill out the redirection table */
        if (indir) {
                int max_queues = tp->num_rx_rings;

                /* Verify user input. */
                for (i = 0; i < reta_entries; i++)
                        if (indir[i] >= max_queues)
                                return -EINVAL;

                for (i = 0; i < reta_entries; i++)
                        tp->rss_indir_tbl[i] = indir[i];
        }

        /* Fill out the rss hash key */
        if (key)
                memcpy(tp->rss_key, key, rtl8125_get_rxfh_key_size(dev));

        rtl8125_store_reta(tp);

        rtl8125_store_rss_key(tp);

        return 0;
}

static u32 rtl8125_get_rx_desc_hash(struct rtl8125_private *tp,
                                    struct RxDescV3 *descv3)
{
        return le32_to_cpu(descv3->RxDescNormalDDWord2.RSSResult);
}

#define RXS_8125B_RSS_UDP BIT(9)
#define RXS_8125_RSS_IPV4 BIT(10)
#define RXS_8125_RSS_IPV6 BIT(12)
#define RXS_8125_RSS_TCP BIT(13)
#define RTL8125_RXS_RSS_L3_TYPE_MASK (RXS_8125_RSS_IPV4 | RXS_8125_RSS_IPV6)
#define RTL8125_RXS_RSS_L4_TYPE_MASK (RXS_8125_RSS_TCP | RXS_8125B_RSS_UDP)
void rtl8125_rx_hash(struct rtl8125_private *tp,
                     struct RxDescV3 *descv3,
                     struct sk_buff *skb)
{
        u16 rss_header_info;

        if (!(tp->dev->features & NETIF_F_RXHASH))
                return;

        rss_header_info = le16_to_cpu(descv3->RxDescNormalDDWord2.HeaderInfo);

        if (!(rss_header_info & RTL8125_RXS_RSS_L3_TYPE_MASK))
                return;

        skb_set_hash(skb, rtl8125_get_rx_desc_hash(tp, descv3),
                     (RTL8125_RXS_RSS_L4_TYPE_MASK & rss_header_info) ?
                     PKT_HASH_TYPE_L4 : PKT_HASH_TYPE_L3);
}

void rtl8125_disable_rss(struct rtl8125_private *tp)
{
        RTL_W32(tp, RSS_CTRL_8125, 0x00);
}

void _rtl8125_config_rss(struct rtl8125_private *tp)
{
        _rtl8125_set_rss_hash_opt(tp);

        rtl8125_store_reta(tp);

        rtl8125_store_rss_key(tp);
}

void rtl8125_config_rss(struct rtl8125_private *tp)
{
        if (!tp->EnableRss) {
                rtl8125_disable_rss(tp);
                return;
        }

        _rtl8125_config_rss(tp);
}

void rtl8125_init_rss(struct rtl8125_private *tp)
{
        int i;

        for (i = 0; i < rtl8125_rss_indir_tbl_entries(tp); i++)
                tp->rss_indir_tbl[i] = ethtool_rxfh_indir_default(i, tp->num_rx_rings);

        netdev_rss_key_fill(tp->rss_key, RTL8125_RSS_KEY_SIZE);
}
