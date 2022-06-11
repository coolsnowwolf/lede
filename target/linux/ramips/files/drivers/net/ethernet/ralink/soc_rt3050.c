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

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "esw_rt3050.h"
#include "mdio_rt2880.h"

static const u16 rt5350_reg_table[FE_REG_COUNT] = {
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
	[FE_REG_FE_RST_GL] = 0,
	[FE_REG_FE_DMA_VID_BASE] = 0,
};

static void rt305x_init_data(struct fe_soc_data *data,
			     struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	priv->flags = FE_FLAG_PADDING_64B | FE_FLAG_PADDING_BUG |
		FE_FLAG_CALIBRATE_CLK | FE_FLAG_HAS_SWITCH;
	netdev->hw_features = NETIF_F_SG | NETIF_F_IP_CSUM |
		NETIF_F_RXCSUM | NETIF_F_HW_VLAN_CTAG_TX;
}

static int rt3050_fwd_config(struct fe_priv *priv)
{
	int ret;

	if (ralink_soc != RT305X_SOC_RT3052) {
		ret = fe_set_clock_cycle(priv);
		if (ret)
			return ret;
	}

	fe_fwd_config(priv);
	if (ralink_soc != RT305X_SOC_RT3352)
		fe_w32(FE_PSE_FQFC_CFG_INIT, FE_PSE_FQ_CFG);
	fe_csum_config(priv);

	return 0;
}

static void rt5350_init_data(struct fe_soc_data *data,
			     struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);

	priv->flags = FE_FLAG_HAS_SWITCH;
	netdev->hw_features = NETIF_F_SG | NETIF_F_RXCSUM;
}

static void rt5350_set_mac(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], RT5350_SDM_MAC_ADRH);
	fe_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       RT5350_SDM_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static void rt5350_rxcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(RT5350_SDM_CFG) | (RT5350_SDM_ICS_EN |
				RT5350_SDM_TCS_EN | RT5350_SDM_UCS_EN),
				RT5350_SDM_CFG);
	else
		fe_w32(fe_r32(RT5350_SDM_CFG) & ~(RT5350_SDM_ICS_EN |
				RT5350_SDM_TCS_EN | RT5350_SDM_UCS_EN),
				RT5350_SDM_CFG);
}

static int rt5350_fwd_config(struct fe_priv *priv)
{
	struct net_device *dev = priv_netdev(priv);

	rt5350_rxcsum_config((dev->features & NETIF_F_RXCSUM));

	return 0;
}

static void rt5350_tx_dma(struct fe_tx_dma *txd)
{
	txd->txd4 = 0;
}

static struct fe_soc_data rt3050_data = {
	.init_data = rt305x_init_data,
	.fwd_config = rt3050_fwd_config,
	.switch_init = rt3050_esw_init,
	.pdma_glo_cfg = FE_PDMA_SIZE_8DWORDS,
	.checksum_bit = RX_DMA_L4VALID,
	.rx_int = FE_RX_DONE_INT,
	.tx_int = FE_TX_DONE_INT,
	.status_int = FE_CNT_GDM_AF,
};

static struct fe_soc_data rt5350_data = {
	.init_data = rt5350_init_data,
	.reg_table = rt5350_reg_table,
	.set_mac = rt5350_set_mac,
	.fwd_config = rt5350_fwd_config,
	.switch_init = rt3050_esw_init,
	.tx_dma = rt5350_tx_dma,
	.pdma_glo_cfg = FE_PDMA_SIZE_8DWORDS,
	.checksum_bit = RX_DMA_L4VALID,
	.rx_int = RT5350_RX_DONE_INT,
	.tx_int = RT5350_TX_DONE_INT,
};

const struct of_device_id of_fe_match[] = {
	{ .compatible = "ralink,rt3050-eth", .data = &rt3050_data },
	{ .compatible = "ralink,rt5350-eth", .data = &rt5350_data },
	{},
};

MODULE_DEVICE_TABLE(of, of_fe_match);
