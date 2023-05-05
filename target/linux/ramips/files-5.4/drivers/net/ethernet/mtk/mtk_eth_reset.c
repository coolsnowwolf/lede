/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2022 MediaTek Inc.
 * Author: Henry Yen <henry.yen@mediatek.com>
 */

#include <linux/regmap.h>
#include "mtk_eth_soc.h"
#include "mtk_eth_dbg.h"
#include "mtk_eth_reset.h"

char* mtk_reset_event_name[32] = {
	[MTK_EVENT_FORCE]	= "Force",
	[MTK_EVENT_WARM_CNT]	= "Warm",
	[MTK_EVENT_COLD_CNT]	= "Cold",
	[MTK_EVENT_TOTAL_CNT]	= "Total",
	[MTK_EVENT_FQ_EMPTY]	= "FQ Empty",
	[MTK_EVENT_TSO_FAIL]	= "TSO Fail",
	[MTK_EVENT_TSO_ILLEGAL]	= "TSO Illegal",
	[MTK_EVENT_TSO_ALIGN]	= "TSO Align",
	[MTK_EVENT_RFIFO_OV]	= "RFIFO OV",
	[MTK_EVENT_RFIFO_UF]	= "RFIFO UF",
};

void mtk_reset_event_update(struct mtk_eth *eth, u32 id)
{
	struct mtk_reset_event *reset_event = &eth->reset_event;
	reset_event->count[id]++;
}

int mtk_eth_cold_reset(struct mtk_eth *eth)
{
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
		regmap_write(eth->ethsys, ETHSYS_FE_RST_CHK_IDLE_EN, 0);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		ethsys_reset(eth,  RSTCTRL_ETH | RSTCTRL_FE | RSTCTRL_PPE0 | RSTCTRL_PPE1);
	else
		ethsys_reset(eth,  RSTCTRL_ETH | RSTCTRL_FE | RSTCTRL_PPE0);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
		regmap_write(eth->ethsys, ETHSYS_FE_RST_CHK_IDLE_EN, 0x3ffffff);

	return 0;
}

int mtk_eth_warm_reset(struct mtk_eth *eth)
{
	u32 reset_bits = 0, i = 0, done = 0;
	u32 val1 = 0, val2 = 0, val3 = 0;

	mdelay(100);

	reset_bits |= RSTCTRL_FE;
	regmap_update_bits(eth->ethsys, ETHSYS_RSTCTRL,
			   reset_bits, reset_bits);

	while (i < 1000) {
		regmap_read(eth->ethsys, ETHSYS_RSTCTRL, &val1);
		if (val1 & RSTCTRL_FE)
			break;
		i++;
		udelay(1);
	}

	if (i < 1000) {
		reset_bits = 0;

		if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
			reset_bits |= RSTCTRL_ETH | RSTCTRL_PPE0 | RSTCTRL_PPE1;
		else
			reset_bits |= RSTCTRL_ETH | RSTCTRL_PPE0;

		regmap_update_bits(eth->ethsys, ETHSYS_RSTCTRL,
				   reset_bits, reset_bits);

		udelay(1);
		regmap_read(eth->ethsys, ETHSYS_RSTCTRL, &val2);
		if (!(val2 & reset_bits))
			pr_info("[%s] error val2=0x%x reset_bits=0x%x !\n",
				__func__, val2, reset_bits);
		reset_bits |= RSTCTRL_FE;
		regmap_update_bits(eth->ethsys, ETHSYS_RSTCTRL,
				   reset_bits, ~reset_bits);

		udelay(1);
		regmap_read(eth->ethsys, ETHSYS_RSTCTRL, &val3);
		if (val3 & reset_bits)
			pr_info("[%s] error val3=0x%x reset_bits=0x%x !\n",
				__func__, val3, reset_bits);
		done = 1;
		mtk_reset_event_update(eth, MTK_EVENT_WARM_CNT);
	}

	pr_info("[%s] reset record val1=0x%x, val2=0x%x, val3=0x%x !\n",
		__func__, val1, val2, val3);

	if (!done)
		mtk_eth_cold_reset(eth);

	return 0;
}

u32 mtk_check_reset_event(struct mtk_eth *eth, u32 status)
{
	u32 ret = 0, val = 0;

	if ((status & MTK_FE_INT_FQ_EMPTY) ||
	    (status & MTK_FE_INT_RFIFO_UF) ||
	    (status & MTK_FE_INT_RFIFO_OV) ||
	    (status & MTK_FE_INT_TSO_FAIL) ||
	    (status & MTK_FE_INT_TSO_ALIGN) ||
	    (status & MTK_FE_INT_TSO_ILLEGAL)) {
		while (status) {
			val = ffs((unsigned int)status) - 1;
			mtk_reset_event_update(eth, val);
			status &= ~(1 << val);
		}
		ret = 1;
	}

	if (atomic_read(&force)) {
		mtk_reset_event_update(eth, MTK_EVENT_FORCE);
		ret = 1;
	}

	if (ret) {
		mtk_reset_event_update(eth, MTK_EVENT_TOTAL_CNT);
		mtk_dump_netsys_info(eth);
	}

	return ret;
}

irqreturn_t mtk_handle_fe_irq(int irq, void *_eth)
{
	struct mtk_eth *eth = _eth;
	u32 status = 0, val = 0;

	status = mtk_r32(eth, MTK_FE_INT_STATUS);
	pr_info("[%s] Trigger FE Misc ISR: 0x%x\n", __func__, status);

	while (status) {
		val = ffs((unsigned int)status) - 1;
		status &= ~(1 << val);

		if ((val == MTK_EVENT_FQ_EMPTY) ||
		    (val == MTK_EVENT_TSO_FAIL) ||
		    (val == MTK_EVENT_TSO_ILLEGAL) ||
		    (val == MTK_EVENT_TSO_ALIGN) ||
		    (val == MTK_EVENT_RFIFO_OV) ||
		    (val == MTK_EVENT_RFIFO_UF))
			pr_info("[%s] Detect reset event: %s !\n", __func__,
				mtk_reset_event_name[val]);
	}
	mtk_w32(eth, 0xFFFFFFFF, MTK_FE_INT_STATUS);

	return IRQ_HANDLED;
}

static void mtk_dump_reg(void *_eth, char *name, u32 offset, u32 range)
{
	struct mtk_eth *eth = _eth;
	u32 cur = offset;

	pr_info("\n============ %s ============\n", name);
	while(cur < offset + range) {
		pr_info("0x%x: %08x %08x %08x %08x\n",
			cur, mtk_r32(eth, cur), mtk_r32(eth, cur + 0x4),
			mtk_r32(eth, cur + 0x8), mtk_r32(eth, cur + 0xc));
		cur += 0x10;
	}
}

void mtk_dump_netsys_info(void *_eth)
{
	struct mtk_eth *eth = _eth;

	mtk_dump_reg(eth, "FE", 0x0, 0x500);
	mtk_dump_reg(eth, "ADMA", PDMA_BASE, 0x300);
	mtk_dump_reg(eth, "QDMA", QDMA_BASE, 0x400);
	mtk_dump_reg(eth, "WDMA", WDMA_BASE(0), 0x600);
	mtk_dump_reg(eth, "PPE", 0x2200, 0x200);
	mtk_dump_reg(eth, "GMAC", 0x10000, 0x300);
}

void mtk_dma_monitor(struct timer_list *t)
{
	struct mtk_eth *eth = from_timer(eth, t, mtk_dma_monitor_timer);
	static u32 timestamp = 0;
	static u32 err_cnt1 = 0, err_cnt2 = 0, err_cnt3 = 0;
	static u32 prev_wdidx = 0;
	u32 cur_wdidx = mtk_r32(eth, MTK_WDMA_DTX_PTR(0));
	u32 is_wtx_busy = mtk_r32(eth, MTK_WDMA_GLO_CFG(0)) & MTK_TX_DMA_BUSY;
	u32 is_oq_free = ((mtk_r32(eth, MTK_PSE_OQ_STA(0)) & 0x01FF0000) == 0) &&
			 ((mtk_r32(eth, MTK_PSE_OQ_STA(1)) & 0x000001FF) == 0) &&
			 ((mtk_r32(eth, MTK_PSE_OQ_STA(4)) & 0x01FF0000) == 0);
	u32 is_cdm_full =
		!(mtk_r32(eth, MTK_WDMA_TX_DBG_MON0(0)) & MTK_CDM_TXFIFO_RDY);
	u32 is_qfsm_hang = mtk_r32(eth, MTK_QDMA_FSM) != 0;
	u32 is_qfwd_hang = mtk_r32(eth, MTK_QDMA_FWD_CNT) == 0;
	u32 is_qfq_hang = mtk_r32(eth, MTK_QDMA_FQ_CNT) !=
			  ((MTK_DMA_SIZE << 16) | MTK_DMA_SIZE);
	u32 is_oq0_stuck = (mtk_r32(eth, MTK_PSE_OQ_STA(0)) & 0x1FF) != 0;
	u32 is_cdm1_busy = (mtk_r32(eth, MTK_FE_CDM1_FSM) & 0xFFFF0000) != 0;
	u32 is_adma_busy = ((mtk_r32(eth, MTK_ADMA_RX_DBG0) & 0x1F) == 0) &&
			   ((mtk_r32(eth, MTK_ADMA_RX_DBG1) & 0x3F0000) == 0) &&
			   ((mtk_r32(eth, MTK_ADMA_RX_DBG0) & 0x40) == 0);

	if (cur_wdidx == prev_wdidx && is_wtx_busy &&
	    is_oq_free && is_cdm_full) {
		err_cnt1++;
		if (err_cnt1 == 3) {
			pr_info("WDMA CDM Hang !\n");
			pr_info("============== Time: %d ================\n",
				timestamp);
			pr_info("err_cnt1 = %d", err_cnt1);
			pr_info("prev_wdidx = 0x%x	| cur_wdidx = 0x%x\n",
				prev_wdidx, cur_wdidx);
			pr_info("is_wtx_busy = %d | is_oq_free = %d	| is_cdm_full = %d\n",
				is_wtx_busy, is_oq_free, is_cdm_full);
			pr_info("-- -- -- -- -- -- --\n");
			pr_info("WDMA_CTX_PTR = 0x%x\n", mtk_r32(eth, 0x4808));
			pr_info("WDMA_DTX_PTR = 0x%x\n",
				mtk_r32(eth, MTK_WDMA_DTX_PTR(0)));
			pr_info("WDMA_GLO_CFG = 0x%x\n",
				mtk_r32(eth, MTK_WDMA_GLO_CFG(0)));
			pr_info("WDMA_TX_DBG_MON0 = 0x%x\n",
				mtk_r32(eth, MTK_WDMA_TX_DBG_MON0(0)));
			pr_info("PSE_OQ_STA1 = 0x%x\n",
				mtk_r32(eth, MTK_PSE_OQ_STA(0)));
			pr_info("PSE_OQ_STA2 = 0x%x\n",
				mtk_r32(eth, MTK_PSE_OQ_STA(1)));
			pr_info("PSE_OQ_STA5 = 0x%x\n",
				mtk_r32(eth, MTK_PSE_OQ_STA(4)));
			pr_info("==============================\n");

			if ((atomic_read(&reset_lock) == 0) &&
			    (atomic_read(&force) == 0)){
				atomic_inc(&force);
				schedule_work(&eth->pending_work);
			}
		}
	} else if (is_qfsm_hang && is_qfwd_hang) {
		err_cnt2++;
		if (err_cnt2 == 3) {
			pr_info("QDMA Tx Hang !\n");
			pr_info("============== Time: %d ================\n",
				timestamp);
			pr_info("err_cnt2 = %d", err_cnt2);
			pr_info("is_qfsm_hang = %d\n", is_qfsm_hang);
			pr_info("is_qfwd_hang = %d\n", is_qfwd_hang);
			pr_info("is_qfq_hang = %d\n", is_qfq_hang);
			pr_info("-- -- -- -- -- -- --\n");
			pr_info("MTK_QDMA_FSM = 0x%x\n",
				mtk_r32(eth, MTK_QDMA_FSM));
			pr_info("MTK_QDMA_FWD_CNT = 0x%x\n",
				mtk_r32(eth, MTK_QDMA_FWD_CNT));
			pr_info("MTK_QDMA_FQ_CNT = 0x%x\n",
				mtk_r32(eth, MTK_QDMA_FQ_CNT));
			pr_info("==============================\n");

			if ((atomic_read(&reset_lock) == 0) &&
			    (atomic_read(&force) == 0)){
				atomic_inc(&force);
				schedule_work(&eth->pending_work);
			}
		}
	} else if (is_oq0_stuck && is_cdm1_busy && is_adma_busy) {
		err_cnt3++;
		if (err_cnt3 == 3) {
			pr_info("ADMA Rx Hang !\n");
			pr_info("============== Time: %d ================\n",
				timestamp);
			pr_info("err_cnt3 = %d", err_cnt3);
			pr_info("is_oq0_stuck = %d\n", is_oq0_stuck);
			pr_info("is_cdm1_busy = %d\n", is_cdm1_busy);
			pr_info("is_adma_busy = %d\n", is_adma_busy);
			pr_info("-- -- -- -- -- -- --\n");
			pr_info("MTK_PSE_OQ_STA1 = 0x%x\n",
				mtk_r32(eth, MTK_PSE_OQ_STA(0)));
			pr_info("MTK_ADMA_RX_DBG0 = 0x%x\n",
				mtk_r32(eth, MTK_ADMA_RX_DBG0));
			pr_info("MTK_ADMA_RX_DBG1 = 0x%x\n",
				mtk_r32(eth, MTK_ADMA_RX_DBG1));
			pr_info("==============================\n");
			if ((atomic_read(&reset_lock) == 0) &&
			    (atomic_read(&force) == 0)){
				atomic_inc(&force);
				schedule_work(&eth->pending_work);
			}
		}
	} else {
		err_cnt1 = 0;
		err_cnt2 = 0;
		err_cnt3 = 0;
	}

	prev_wdidx = cur_wdidx;
	mod_timer(&eth->mtk_dma_monitor_timer, jiffies + 1 * HZ);
}

void mtk_prepare_reset_fe(struct mtk_eth *eth)
{
	u32 i = 0, val = 0;

	/* Disable NETSYS Interrupt */
	mtk_w32(eth, 0, MTK_FE_INT_ENABLE);
	mtk_w32(eth, 0, MTK_PDMA_INT_MASK);
	mtk_w32(eth, 0, MTK_QDMA_INT_MASK);

	/* Disable Linux netif Tx path */
	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		netif_tx_disable(eth->netdev[i]);
	}

	/* Disable QDMA Tx */
	val = mtk_r32(eth, MTK_QDMA_GLO_CFG);
	mtk_w32(eth, val & ~(MTK_TX_DMA_EN), MTK_QDMA_GLO_CFG);

	/* Power down sgmii */
	regmap_read(eth->sgmii->regmap[0], SGMSYS_QPHY_PWR_STATE_CTRL, &val);
	val |= SGMII_PHYA_PWD;
	regmap_write(eth->sgmii->regmap[0], SGMSYS_QPHY_PWR_STATE_CTRL, val);
	regmap_read(eth->sgmii->regmap[1], SGMSYS_QPHY_PWR_STATE_CTRL, &val);
	val |= SGMII_PHYA_PWD;
	regmap_write(eth->sgmii->regmap[1], SGMSYS_QPHY_PWR_STATE_CTRL, val);

	/* Force link down GMAC */
	val = mtk_r32(eth, MTK_MAC_MCR(0));
	mtk_w32(eth, val & ~(MAC_MCR_FORCE_LINK), MTK_MAC_MCR(0));
	val = mtk_r32(eth, MTK_MAC_MCR(1));
	mtk_w32(eth, val & ~(MAC_MCR_FORCE_LINK), MTK_MAC_MCR(1));

	/* Disable GMAC Rx */
	val = mtk_r32(eth, MTK_MAC_MCR(0));
	mtk_w32(eth, val & ~(MAC_MCR_RX_EN), MTK_MAC_MCR(0));
	val = mtk_r32(eth, MTK_MAC_MCR(1));
	mtk_w32(eth, val & ~(MAC_MCR_RX_EN), MTK_MAC_MCR(1));

	/* Enable GDM drop */
	mtk_gdm_config(eth, MTK_GDMA_DROP_ALL);

	/* Disable ADMA Rx */
	val = mtk_r32(eth, MTK_PDMA_GLO_CFG);
	mtk_w32(eth, val & ~(MTK_RX_DMA_EN), MTK_PDMA_GLO_CFG);
}

void mtk_prepare_reset_ppe(struct mtk_eth *eth, u32 ppe_id)
{
	u32 i = 0, poll_time = 5000, val;

	/* Disable KA */
	mtk_m32(eth, MTK_PPE_KA_CFG_MASK, 0, MTK_PPE_TB_CFG(ppe_id));
	mtk_m32(eth, MTK_PPE_NTU_KA_MASK, 0, MTK_PPE_BIND_LMT_1(ppe_id));
	mtk_w32(eth, 0, MTK_PPE_KA(ppe_id));
	mdelay(10);

	/* Set KA timer to maximum */
	mtk_m32(eth, MTK_PPE_NTU_KA_MASK, (0xFF << 16), MTK_PPE_BIND_LMT_1(ppe_id));
	mtk_w32(eth, 0xFFFFFFFF, MTK_PPE_KA(ppe_id));

	/* Set KA tick select */
	mtk_m32(eth, MTK_PPE_TICK_SEL_MASK, (0x1 << 24), MTK_PPE_TB_CFG(ppe_id));
	mtk_m32(eth, MTK_PPE_KA_CFG_MASK, (0x3 << 12), MTK_PPE_TB_CFG(ppe_id));
	mdelay(10);

	/* Disable scan mode */
	mtk_m32(eth, MTK_PPE_SCAN_MODE_MASK, 0, MTK_PPE_TB_CFG(ppe_id));
	mdelay(10);

	/* Check PPE idle */
	while (i++ < poll_time) {
		val = mtk_r32(eth, MTK_PPE_GLO_CFG(ppe_id));
		if (!(val & MTK_PPE_BUSY))
			break;
		mdelay(1);
	}

	if (i >= poll_time) {
		pr_info("[%s] PPE keeps busy !\n", __func__);
		mtk_dump_reg(eth, "FE", 0x0, 0x500);
		mtk_dump_reg(eth, "PPE", 0x2200, 0x200);
	}
}

static int mtk_eth_netdevice_event(struct notifier_block *unused,
				   unsigned long event, void *ptr)
{
	switch (event) {
	case MTK_WIFI_RESET_DONE:
		complete(&wait_ser_done);
		break;
	default:
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block mtk_eth_netdevice_nb __read_mostly = {
	.notifier_call = mtk_eth_netdevice_event,
};
