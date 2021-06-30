/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/if_vlan.h>
#include <linux/of_net.h>

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"
#include "mt7530.h"
#include "mdio.h"

#define MT7620A_CDMA_CSG_CFG	0x400
#define MT7621_CDMP_IG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x00)
#define MT7621_CDMP_EG_CTRL	(MT7620A_CDMA_CSG_CFG + 0x04)
#define MT7621_RESET_FE		BIT(6)
#define MT7621_L4_VALID		BIT(24)

#define MT7621_TX_DMA_UDF	BIT(19)
#define MT7621_TX_DMA_FPORT	BIT(25)

#define CDMA_ICS_EN		BIT(2)
#define CDMA_UCS_EN		BIT(1)
#define CDMA_TCS_EN		BIT(0)

#define GDMA_ICS_EN		BIT(22)
#define GDMA_TCS_EN		BIT(21)
#define GDMA_UCS_EN		BIT(20)

/* frame engine counters */
#define MT7621_REG_MIB_OFFSET	0x2000
#define MT7621_PPE_AC_BCNT0	(MT7621_REG_MIB_OFFSET + 0x00)
#define MT7621_GDM1_TX_GBCNT	(MT7621_REG_MIB_OFFSET + 0x400)
#define MT7621_GDM2_TX_GBCNT	(MT7621_GDM1_TX_GBCNT + 0x40)

#define GSW_REG_GDMA1_MAC_ADRL	0x508
#define GSW_REG_GDMA1_MAC_ADRH	0x50C

#define MT7621_FE_RST_GL	(FE_FE_OFFSET + 0x04)
#define MT7620_FE_INT_STATUS2	(FE_FE_OFFSET + 0x08)

/* FE_INT_STATUS reg on mt7620 define CNT_GDM1_AF at BIT(29)
 * but after test it should be BIT(13).
 */
#define MT7620_FE_GDM1_AF	BIT(13)
#define MT7621_FE_GDM1_AF	BIT(28)
#define MT7621_FE_GDM2_AF	BIT(29)

static const u16 mt7621_reg_table[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = RT5350_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = RT5350_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = RT5350_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = RT5350_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = RT5350_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = RT5350_TX_CTX_IDX0,
	[FE_REG_TX_DTX_IDX0] = RT5350_TX_DTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = RT5350_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = RT5350_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = RT5350_RX_CALC_IDX0,
	[FE_REG_RX_DRX_IDX0] = RT5350_RX_DRX_IDX0,
	[FE_REG_FE_INT_ENABLE] = RT5350_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = RT5350_FE_INT_STATUS,
	[FE_REG_FE_DMA_VID_BASE] = 0,
	[FE_REG_FE_COUNTER_BASE] = MT7621_GDM1_TX_GBCNT,
	[FE_REG_FE_RST_GL] = MT7621_FE_RST_GL,
	[FE_REG_FE_INT_STATUS2] = MT7620_FE_INT_STATUS2,
};

static int mt7621_gsw_config(struct fe_priv *priv)
{
	if (priv->mii_bus &&  mdiobus_get_phy(priv->mii_bus, 0x1f))
		mt7530_probe(priv->dev, NULL, priv->mii_bus, 1);

	return 0;
}

static void mt7621_fe_reset(void)
{
	fe_reset(MT7621_RESET_FE);
}

static void mt7621_rxcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) | (GDMA_ICS_EN |
					GDMA_TCS_EN | GDMA_UCS_EN),
				MT7620A_GDMA1_FWD_CFG);
	else
		fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~(GDMA_ICS_EN |
					GDMA_TCS_EN | GDMA_UCS_EN),
				MT7620A_GDMA1_FWD_CFG);
}

static void mt7621_rxvlan_config(bool enable)
{
	if (enable)
		fe_w32(1, MT7621_CDMP_EG_CTRL);
	else
		fe_w32(0, MT7621_CDMP_EG_CTRL);
}

static int mt7621_fwd_config(struct fe_priv *priv)
{
	struct net_device *dev = priv_netdev(priv);

	fe_w32(fe_r32(MT7620A_GDMA1_FWD_CFG) & ~0xffff,
	       MT7620A_GDMA1_FWD_CFG);

	/* mt7621 doesn't have txcsum config */
	mt7621_rxcsum_config((dev->features & NETIF_F_RXCSUM));
	mt7621_rxvlan_config(dev->features & NETIF_F_HW_VLAN_CTAG_RX);

	return 0;
}

static void mt7621_tx_dma(struct fe_tx_dma *txd)
{
	txd->txd4 = MT7621_TX_DMA_FPORT;
}

static void mt7621_init_data(struct fe_soc_data *data,
			     struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	priv->flags = FE_FLAG_PADDING_64B | FE_FLAG_RX_2B_OFFSET |
		FE_FLAG_RX_SG_DMA | FE_FLAG_NAPI_WEIGHT |
		FE_FLAG_HAS_SWITCH | FE_FLAG_JUMBO_FRAME;

	netdev->hw_features = NETIF_F_IP_CSUM | NETIF_F_RXCSUM |
		NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX |
		NETIF_F_SG | NETIF_F_IPV6_CSUM |
		NETIF_F_TSO_MANGLEID;
}

static void mt7621_set_mac(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], GSW_REG_GDMA1_MAC_ADRH);
	fe_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       GSW_REG_GDMA1_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static struct fe_soc_data mt7621_data = {
	.init_data = mt7621_init_data,
	.reset_fe = mt7621_fe_reset,
	.set_mac = mt7621_set_mac,
	.fwd_config = mt7621_fwd_config,
	.tx_dma = mt7621_tx_dma,
	.switch_init = mtk_gsw_init,
	.switch_config = mt7621_gsw_config,
	.reg_table = mt7621_reg_table,
	.pdma_glo_cfg = FE_PDMA_SIZE_16DWORDS,
	.rx_int = RT5350_RX_DONE_INT,
	.tx_int = RT5350_TX_DONE_INT,
	.status_int = (MT7621_FE_GDM1_AF | MT7621_FE_GDM2_AF),
	.checksum_bit = MT7621_L4_VALID,
	.has_carrier = mt7620_has_carrier,
	.mdio_read = mt7620_mdio_read,
	.mdio_write = mt7620_mdio_write,
	.mdio_adjust_link = mt7620_mdio_link_adjust,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "mediatek,ralink-mt7621-eth", .data = &mt7621_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
