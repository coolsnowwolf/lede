/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#include <linux/mm.h>
#include <linux/io.h>
#include <asm/page.h>
#include <asm/cache.h>
#include <asm/board/board_config.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <common/topaz_platform.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_emac.h"
#include "qdpc_ruby.h"

#define REG_WRITE(x,y)        (writel((y),(x)))
#define REG_READ(x)  	      (readl(x))
#define QDPC_PHY1_ADDRESS      (31)
#define QDPC_PHY0_ADDRESS      (24)

#define QDPC_NUM_DESC	      QDPC_DESC_RING_SIZE
#define QDPC_WRITE_INDEX_LEN  (32 << 12)

/*
 * Gather EMAC DMA rx lost statistics for netdebug function.
 * Any module used for netdebug backhaul must define this function.
 */
uint32_t qtn_eth_rx_lost_get(struct net_device *dev)
{
	struct qdpc_priv *priv = netdev_priv(dev);
	uint32_t dsbase = priv->pktq.pkt_dsq.pd_rx_basereg;
	uint32_t usbase = priv->pktq.pkt_usq.pd_rx_basereg;

	uint32_t ds_lost = REG_READ(dsbase + EMAC_DMAMFC) & 0x7fffffff;
	uint32_t us_lost = REG_READ(usbase + EMAC_DMAMFC) & 0x7fffffff;

	return (ds_lost + us_lost);
}
EXPORT_SYMBOL(qtn_eth_rx_lost_get);

void qdpc_flush_and_inv_dcache_range(unsigned long start, unsigned long end)
{
	unsigned long flags, dc_ctrl;

	start &= DCACHE_LINE_MASK;
	local_irq_save(flags);
	/* Set the Invalidate mode to FLUSH BEFORE INV */
	dc_ctrl = read_new_aux_reg(ARC_REG_DC_CTRL);
	write_new_aux_reg(ARC_REG_DC_CTRL, dc_ctrl | INV_MODE_FLUSH);
	/* Invoke Cache INV CMD */
	while (end > start) {
		write_new_aux_reg(ARC_REG_DC_IVDL, start);
		start = start + ARC_DCACHE_LINE_LEN;
	}
	/* wait for the flush to complete, poll on the FS Bit */
	while (read_new_aux_reg(ARC_REG_DC_CTRL) & DC_FLUSH_STATUS_BIT) ;
	/* Switch back the DISCARD ONLY Invalidate mode */
	write_new_aux_reg(ARC_REG_DC_CTRL, dc_ctrl & ~INV_MODE_FLUSH);
	local_irq_restore(flags);
}

void phy_write(u32 phyaddr, u32 regn, u16 data)
{
	u32 timeout = 0x10000;
	/* write data */
	REG_WRITE(PHY_MDIODATA, data);
	/* issue write */
	REG_WRITE(PHY_MDIOCTRL,
		(phyaddr << 0) | (regn << 5) | (0 << 10) | (1 << 15));
	/* wait for write completion */
	while (REG_READ(PHY_MDIOCTRL) & (1 << 15)) {
		if (timeout-- == 0) {
			PRINT_ERROR("MDIO timeout\n");
			break;
		}
	}
}

int32_t qdpc_emac_getconfig(struct qdpc_priv *priv)
{
	int pcie_config = 0;

	/* EMAC DMA engine assignment */
	priv->pktq.pkt_dsq.pd_tx_basereg = EMAC0_ADDR;
#ifdef QDPC_PHY_CROSS_MODE
	priv->pktq.pkt_dsq.pd_rx_basereg = EMAC1_ADDR;
#else
	priv->pktq.pkt_dsq.pd_rx_basereg = EMAC0_ADDR;
#endif
	priv->pktq.pkt_usq.pd_tx_basereg = EMAC1_ADDR;
#ifdef QDPC_PHY_CROSS_MODE
	priv->pktq.pkt_usq.pd_rx_basereg = EMAC0_ADDR;
#else
	priv->pktq.pkt_usq.pd_rx_basereg = EMAC1_ADDR;
#endif
	priv->pktq.pkt_dsq.pd_src_busaddr =
					priv->shmem_busaddr + priv->shmem->eps_dsdma_desc;
	priv->pktq.pkt_usq.pd_dst_busaddr =
					priv->shmem_busaddr + priv->shmem->eps_usdma_desc;

	/* check for Use PHY Loopback configuration */
	get_board_config(BOARD_CFG_PCIE, &pcie_config);
	if ((pcie_config & PCIE_USE_PHY_LOOPBK) == PCIE_USE_PHY_LOOPBK)
	{
                priv->mii_pllclk = 0x8f8f8f8f;
	} else {
#ifdef CVM_HOST
                priv->mii_pllclk = 0x8f808f8f;
#else
                priv->mii_pllclk = 0x8f808f80;
#endif
	}

        return 0;
}

void qdpc_emac_startdma(qdpc_pdring_t *pktq)
{
	REG_WRITE(pktq->pd_rx_basereg + EMAC_DMARPD, 1);
	REG_WRITE(pktq->pd_tx_basereg + EMAC_DMATPD, 1);

}

void qdpc_emac_disable(struct qdpc_priv *priv)
{
	/* Disable EMAC1 TX poll demand */
	REG_WRITE(priv->pktq.pkt_usq.pd_tx_basereg + EMAC_DMATAPC, 0x0);

	/* Disable Interrupts */
	REG_WRITE(priv->pktq.pkt_dsq.pd_rx_basereg + EMAC_DMAINTEN, ~DmaAllInts);
	REG_WRITE(priv->pktq.pkt_usq.pd_rx_basereg + EMAC_DMAINTEN, ~DmaAllInts);
}

void qdpc_emac_enable(struct qdpc_priv *priv)
{
	uint32_t tmp;
	uint32_t dsdma_base = priv->pktq.pkt_dsq.pd_tx_basereg;
	uint32_t usdma_base = priv->pktq.pkt_usq.pd_tx_basereg;

	/* EMAC0 Interrupts:
	  *  DmaRxDone
	  *  EMAC0_ERROR (DmaRxMissedFrame | DmaNoTxDesc | DmaNoRxDesc)
	  *  DmaMacInterrupt
	  *  MAC Interupts : 
	  *		MacUnderrun
	  */

	REG_WRITE(dsdma_base + EMAC_MACINTEN, QDPC_MAC_DS_INTR);
	REG_WRITE(dsdma_base + EMAC_DMAINTEN, QDPC_DMA_DS_INTR);

	/* EMAC1 Interrupts:
	  *  DmaRxDone
	  */
	REG_WRITE(usdma_base + EMAC_DMAINTEN, QDPC_DMA_US_INTR);
	REG_WRITE(dsdma_base + EMAC_MACINTEN, QDPC_MAC_US_INTR);

	REG_WRITE(dsdma_base + EMAC_DMACTRL, DmaStartTx | DmaStartRx);
	REG_WRITE(usdma_base + EMAC_DMACTRL, DmaStartTx | DmaStartRx);

	/* EMAC1 TX poll demand */
	REG_WRITE(usdma_base + EMAC_DMATAPC, QDPC_DMA_AUTO_POLLING_CNT);

	/* clear missed frames and stopped flush */
	tmp = REG_READ(dsdma_base + EMAC_DMAMFC);
	tmp = REG_READ(dsdma_base + EMAC_DMASFC);

	tmp = REG_READ(usdma_base + EMAC_DMAMFC);
	tmp = REG_READ(usdma_base + EMAC_DMASFC);

}

void qdpc_emac_phyinit(struct qdpc_priv *priv)
{
	int rgmii_timing = priv->mii_pllclk;
#if defined(CONFIG_RUBY_PCIE_HAVE_PHY)
	int phy_setting_mask = 0;
#endif

	REG_WRITE(RUBY_SYS_CTL_MASK, RUBY_SYS_CTL_MASK_MII);
	REG_WRITE(RUBY_SYS_CTL_CTRL, 0);
	REG_WRITE(RUBY_SYS_CTL_MASK, RUBY_SYS_CTL_MASK_GMII0_TXCLK);
	/* 2 is 1000MB */
	REG_WRITE(RUBY_SYS_CTL_CTRL, RUBY_SYS_CTL_MASK_GMII0_1000M);
	REG_WRITE(RUBY_SYS_CTL_GMII_CLKDLL, rgmii_timing);
	REG_WRITE(RUBY_SYS_CTL_MASK, RUBY_SYS_CTL_MASK_GMII1_TXCLK);
	/* 2 is 1000MB */
	REG_WRITE(RUBY_SYS_CTL_CTRL,RUBY_SYS_CTL_MASK_GMII1_1000M);
	REG_WRITE(RUBY_SYS_CTL_GMII_CLKDLL, rgmii_timing);

	REG_WRITE(RUBY_SYS_CTL_CPU_VEC_MASK, RUBY_SYS_CTL_RESET_ENET0 | RUBY_SYS_CTL_RESET_ENET1);
	REG_WRITE(RUBY_SYS_CTL_CPU_VEC, 0);
	REG_WRITE(RUBY_SYS_CTL_CPU_VEC, RUBY_SYS_CTL_RESET_ENET0 | RUBY_SYS_CTL_RESET_ENET1);

#if defined(CONFIG_RUBY_PCIE_HAVE_PHY)
	/* EMAC0 RGMII PLL Setup 1GB FDX loopback */
	phy_write(QDPC_PHY0_ADDRESS, PHY_MODE_CTL, PHY_MODE_CTL_RESET);
	udelay(140);

	phy_setting_mask |= PHY_MODE_CTL_100MB |
			PHY_MODE_CTL_FULL_DUPLEX | PHY_MODE_CTL_LOOPBACK;
	phy_write(QDPC_PHY0_ADDRESS, PHY_MODE_CTL, phy_setting_mask);

	/* EMAC1 RGMII Setup  Setup 1GB FDX loopback*/
	phy_write(QDPC_PHY1_ADDRESS, PHY_MODE_CTL, PHY_MODE_CTL_RESET);
	udelay(140);
	phy_write(QDPC_PHY1_ADDRESS, PHY_MODE_CTL, phy_setting_mask);
#endif
}

void qdpc_emac_reset(struct qdpc_priv *priv)
{
	uint32_t dsbase = priv->pktq.pkt_dsq.pd_tx_basereg;
	uint32_t usbase = priv->pktq.pkt_usq.pd_tx_basereg;

	/* Reset EMACs */
	REG_WRITE(dsbase + EMAC_DMACONFIG, DmaSoftReset);
	REG_WRITE(usbase + EMAC_DMACONFIG, DmaSoftReset);
	REG_WRITE(dsbase + EMAC_DMACONFIG, 0);
	REG_WRITE(usbase + EMAC_DMACONFIG, 0);

	/* Disable DMA */
	REG_WRITE(dsbase + EMAC_DMACTRL, 0);
	REG_WRITE(usbase + EMAC_DMACTRL, 0);

	/* Disable Interrupts */
	REG_WRITE(dsbase + EMAC_DMAINTEN, ~DmaAllInts);
	REG_WRITE(usbase + EMAC_DMAINTEN, ~DmaAllInts);
}

int32_t qdpc_usdma_init(struct qdpc_priv *priv, size_t ringsize)
{
	qdpc_pdring_t *usq= &priv->pktq.pkt_usq;
	uint32_t regbase = usq->pd_tx_basereg;
	uint32_t rx_regbase = usq->pd_rx_basereg;

	/* Setup Downstream  DMA - EMAC1 */
	qdpc_init_txq(priv, ringsize);

	/* Write start of RX ring to EMAC1 */
	REG_WRITE(rx_regbase + EMAC_DMARBA, usq->pd_dst_busaddr);

	/* Write start of TX ring (on host ) to EMAC1 */
	REG_WRITE(regbase + EMAC_DMATBA, usq->pd_src_busaddr);

	/* Setup DMA config, bring DMA controller out of reset */
	REG_WRITE(regbase + EMAC_DMACONFIG, Dma64BitMode | DmaWait4Done | Dma16WordBurst);

	printk("Upstream DMA: s:0x%x d:0x%x\n", usq->pd_src_busaddr, usq->pd_dst_busaddr);

	return 0;
}

int32_t qdpc_dsdma_init(struct qdpc_priv *priv, size_t ringsize)
{
	qdpc_pdring_t *dsq= &priv->pktq.pkt_dsq;
	uint32_t regbase = dsq->pd_tx_basereg;
	uint32_t rx_regbase = dsq->pd_rx_basereg;

	/* Setup Downstream  DMA - EMAC0 */
	qdpc_init_rxq(priv, ringsize);

	/* Write start of RX ring to EMAC0 */
	REG_WRITE(rx_regbase + EMAC_DMARBA, dsq->pd_dst_busaddr);

	/* Write start of TX ring (on host ) to EMAC0 */
	REG_WRITE(regbase + EMAC_DMATBA, dsq->pd_src_busaddr);

	/* Setup DMA config, bring DMA controller out of reset */
	REG_WRITE(regbase + EMAC_DMACONFIG, Dma64BitMode | DmaWait4Done | Dma16WordBurst);

	printk("downstream DMA: s:0x%x d:0x%x\n", dsq->pd_src_busaddr, dsq->pd_dst_busaddr);

	return 0;
}

void qdpc_emac_initcommon(uint32_t regbase)
{
	/* PHY mode */
	REG_WRITE(regbase + EMAC_MACGCTRL, EMAC_100MBPS | EMAC_FD);

	/* Max Frame Length */
	REG_WRITE(regbase + EMAC_MACMFS, QDPC_MAX_FRAMESZ);
	REG_WRITE(regbase + EMAC_MACTJS, QDPC_MAX_JABBER);
	REG_WRITE(regbase + EMAC_MACRJS, QDPC_MAX_JABBER);

	/* Flow Control Decode and Generation Disabled */
	//REG_WRITE(regbase + EMAC_MACFCPTV, 0);
	REG_WRITE(regbase + EMAC_MACFCCTRL, MacFlowDecodeEnable |
			MacFlowGenerationEnable | MacAutoFlowGenerationEnable |
			MacFlowMulticastMode | MacBlockPauseFrames);

	/* !!! FIXME - whether or not we need this depends on whether
	 * the auto-pause generation uses it.  The auto function may just
	 * use 0xffff val to stop sending & then 0 to restart it.
	 */
	REG_WRITE(regbase + EMAC_MACFCPTV, 100);
	REG_WRITE(regbase + EMAC_MACFCPTVH, 200);
	REG_WRITE(regbase + EMAC_MACFCPTVL, 0);

	/* source mac: 00:26:86:00:00:26 */
	REG_WRITE(regbase + EMAC_MACFCSAH, 0x0026);
	REG_WRITE(regbase + EMAC_MACFCSAM, 0x8600);
	REG_WRITE(regbase + EMAC_MACFCSAL, 0x0026);

	REG_WRITE(regbase + EMAC_DMAFCT, MacFCHighThreshold | MacFCLowThreshold);

	REG_WRITE(regbase + EMAC_DMATAPC, 0x00);
	REG_WRITE(regbase + EMAC_MACACTRL, MacPromiscuous);
	REG_WRITE(regbase + EMAC_MACTCTRL, MacTxEnable | MacTxIFG256 | MacTxDisableFCSInsertion);
}

void qdpc_emac_dsinit(struct qdpc_priv *priv)
{
	uint32_t dsbase = priv->pktq.pkt_dsq.pd_tx_basereg;
	/* Init Downstream DMA ring */
	qdpc_dsdma_init(priv, QDPC_DESC_RING_SIZE);
	/* Specific flow, buffering and timing inits */
	REG_WRITE(dsbase + EMAC_MACTFIFOAF, 512 - 8);
	REG_WRITE(dsbase + EMAC_MACTPST, 1518);
	REG_WRITE(dsbase + EMAC_MACRPST, 0);

	/* Enable DMA Rx Transfer Done interrupt mitigation control */
	REG_WRITE(dsbase + EMAC_DMARDIMC, (QDPC_DMA_TX_IMC_EN |
		QDPC_DMA_TX_IMC_DELAY | QDPC_DMA_TX_IMC_NFRAMES(QDPC_DMA_INT_MITIGATION_NUM)));
	REG_WRITE(dsbase + EMAC_MACRCTRL, MacRxEnable | MacRxDisableFcs | MacRxStoreAndForward);

	/* Flow Control Decode and Generation Disabled */
	REG_WRITE(dsbase + EMAC_MACFCPTV, 0);

	/* Common init */
	qdpc_emac_initcommon(dsbase);
}


void qdpc_emac_usinit(struct qdpc_priv *priv)
{
	uint32_t usbase = priv->pktq.pkt_usq.pd_tx_basereg;
	/* Init UPstream DMA ring */
	qdpc_usdma_init(priv, QDPC_DESC_USRING_SIZE);
	/* Specific flow, buffering and timing inits */
	REG_WRITE(usbase + EMAC_MACTFIFOAF, 512 - 8);
	REG_WRITE(usbase + EMAC_MACTPST, 1518);
	REG_WRITE(usbase + EMAC_MACRPST, 0);

	/* Enable DMA Rx Transfer Done interrupt mitigation control */
	REG_WRITE(usbase + EMAC_DMARDIMC,(QDPC_DMA_TX_IMC_EN |
		QDPC_DMA_TX_IMC_DELAY | QDPC_DMA_TX_IMC_NFRAMES(QDPC_DMA_INT_MITIGATION_NUM)));
	REG_WRITE(usbase + EMAC_MACRCTRL, MacRxEnable | MacRxDisableFcs | MacRxStoreAndForward);

	/* Enable Flow Control Decode and Generation  */
	REG_WRITE(usbase + EMAC_MACFCPTV, 0);

	/* Common init */
	qdpc_emac_initcommon(usbase);
}

int32_t qdpc_emac_init(struct qdpc_priv *priv)
{

	/* Get configration for upstream and downstream DMA engines */
	qdpc_emac_getconfig(priv);

	/* Setup PHYs */
	qdpc_emac_phyinit(priv);

	/* Reset the EMACs */
	qdpc_emac_reset(priv);

	/* Initialize the downstream and upstream DMA engines */
	qdpc_emac_dsinit(priv);
	qdpc_emac_usinit(priv);

	return SUCCESS;
}
 /* USE_EMAC_DMA */
