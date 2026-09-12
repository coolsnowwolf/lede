// SPDX-License-Identifier: GPL-2.0-only
/**
 * 1588 PTP support for Phytium GMAC device.
 *
 * Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd.
 *
 * Author: Wenting Song <songwenting@phytium.com>
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/time64.h>
#include <linux/ptp_classify.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/net_tstamp.h>
#include <linux/circ_buf.h>
#include <linux/spinlock.h>
#include "phytmac.h"
#include "phytmac_ptp.h"

bool phytmac_ptp_one_step(struct sk_buff *skb)
{
	struct ptp_header *hdr;
	unsigned int ptp_class;
	u8 msgtype;

	/* No need to parse packet if PTP TS is not involved */
	if (likely(!(skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)))
		goto not_oss;

	/* Identify and return whether PTP one step sync is being processed */
	ptp_class = ptp_classify_raw(skb);
	if (ptp_class == PTP_CLASS_NONE)
		goto not_oss;

	hdr = ptp_parse_header(skb, ptp_class);
	if (!hdr)
		goto not_oss;

	if (hdr->flag_field[0] & 0x2)
		goto not_oss;

	msgtype = ptp_get_msgtype(hdr, ptp_class);
	if (msgtype == PTP_MSGTYPE_SYNC)
		return true;

not_oss:
	return false;
}

int phytmac_ptp_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts)
{
	struct phytmac *pdata = container_of(ptp, struct phytmac, ptp_clock_info);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	unsigned long flags;

	spin_lock_irqsave(&pdata->ts_clk_lock, flags);
	hw_if->get_time(pdata, ts);
	spin_unlock_irqrestore(&pdata->ts_clk_lock, flags);

	return 0;
}

int phytmac_ptp_settime(struct ptp_clock_info *ptp,
			const struct timespec64 *ts)
{
	struct phytmac *pdata = container_of(ptp, struct phytmac, ptp_clock_info);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	unsigned long flags;

	spin_lock_irqsave(&pdata->ts_clk_lock, flags);
	hw_if->set_time(pdata, ts->tv_sec, ts->tv_nsec);
	spin_unlock_irqrestore(&pdata->ts_clk_lock, flags);

	return 0;
}

int phytmac_ptp_adjfine(struct ptp_clock_info *ptp, long scaled_ppm)
{
	struct phytmac *pdata = container_of(ptp, struct phytmac, ptp_clock_info);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	bool negative = false;

	if (scaled_ppm < 0) {
		negative = true;
		scaled_ppm = -scaled_ppm;
	}

	hw_if->adjust_fine(pdata, scaled_ppm, negative);
	return 0;
}

int phytmac_ptp_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	struct phytmac *pdata = container_of(ptp, struct phytmac, ptp_clock_info);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	int negative = 0;

	if (delta < 0) {
		negative = 1;
		delta = -delta;
	}

	spin_lock_irq(&pdata->ts_clk_lock);
	hw_if->adjust_time(pdata, delta, negative);
	spin_unlock_irq(&pdata->ts_clk_lock);

	return 0;
}

int phytmac_ptp_enable(struct ptp_clock_info *ptp,
		       struct ptp_clock_request *rq, int on)
{
	return -EOPNOTSUPP;
}

void phytmac_ptp_init_timer(struct phytmac *pdata)
{
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	u32 rem = 0;
	u64 adj;

	pdata->ts_rate = hw_if->get_ts_rate(pdata);
	pdata->ts_incr.ns = div_u64_rem(NSEC_PER_SEC, pdata->ts_rate, &rem);
	if (rem) {
		adj = rem;
		adj <<= 24;
		pdata->ts_incr.sub_ns = div_u64(adj, pdata->ts_rate);
	} else {
		pdata->ts_incr.sub_ns = 0;
	}
}

void phytmac_ptp_rxstamp(struct phytmac *pdata, struct sk_buff *skb,
			 struct phytmac_dma_desc *desc)
{
	struct skb_shared_hwtstamps *shhwtstamps = skb_hwtstamps(skb);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	struct timespec64 ts;

	if (hw_if->ts_valid(pdata, desc, PHYTMAC_RX)) {
		hw_if->get_timestamp(pdata, desc->desc4, desc->desc5, &ts);
		memset(shhwtstamps, 0, sizeof(struct skb_shared_hwtstamps));
		shhwtstamps->hwtstamp = ktime_set(ts.tv_sec, ts.tv_nsec);
	}
}

int phytmac_ptp_txstamp(struct phytmac_queue *queue, struct sk_buff *skb,
			struct phytmac_dma_desc *desc)
{
	struct phytmac *pdata = queue->pdata;
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	struct timespec64 ts;
	struct skb_shared_hwtstamps shhwtstamps;

	if (queue->pdata->ts_config.tx_type == TS_DISABLED)
		return -EOPNOTSUPP;

	if (!hw_if->ts_valid(pdata, desc, PHYTMAC_TX))
		return -EINVAL;

	skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
	hw_if->get_timestamp(pdata, desc->desc4, desc->desc5, &ts);
	memset(&shhwtstamps, 0, sizeof(shhwtstamps));
	shhwtstamps.hwtstamp = ktime_set(ts.tv_sec, ts.tv_nsec);
	skb_tstamp_tx(skb, &shhwtstamps);

	return 0;
}

int phytmac_ptp_register(struct phytmac *pdata)
{
	pdata->ptp_clock_info.owner = THIS_MODULE;
	snprintf(pdata->ptp_clock_info.name, 16, "%s", pdata->ndev->name);
	pdata->ptp_clock_info.max_adj = 64000000; /* In PPB */
	pdata->ptp_clock_info.n_alarm = 0;
	pdata->ptp_clock_info.n_ext_ts = 0;
	pdata->ptp_clock_info.n_per_out = 0;
	pdata->ptp_clock_info.pps = 1;
	pdata->ptp_clock_info.adjfine = phytmac_ptp_adjfine;
	pdata->ptp_clock_info.adjtime = phytmac_ptp_adjtime;
	pdata->ptp_clock_info.gettime64 = phytmac_ptp_gettime;
	pdata->ptp_clock_info.settime64 = phytmac_ptp_settime;
	pdata->ptp_clock_info.enable = phytmac_ptp_enable;
	pdata->ptp_clock = ptp_clock_register(&pdata->ptp_clock_info, pdata->dev);
	if (IS_ERR_OR_NULL(pdata->ptp_clock)) {
		dev_err(pdata->dev, "ptp_clock_register failed %lu\n",
			PTR_ERR(pdata->ptp_clock));
		return -EINVAL;
	}

	return 0;
}

void phytmac_ptp_unregister(struct phytmac *pdata)
{
	struct phytmac_hw_if *hw_if = pdata->hw_if;

	if (pdata->ptp_clock)
		ptp_clock_unregister(pdata->ptp_clock);
	pdata->ptp_clock = NULL;

	hw_if->clear_time(pdata);

	dev_info(pdata->dev, "phytmac ptp clock unregistered.\n");
}

void phytmac_ptp_init(struct net_device *ndev)
{
	struct phytmac *pdata = netdev_priv(ndev);
	struct phytmac_hw_if *hw_if = pdata->hw_if;

	phytmac_ptp_init_timer(pdata);

	hw_if->init_ts_hw(pdata);

	dev_info(pdata->dev, "phytmac ptp clock init success.\n");
}

int phytmac_ptp_get_ts_config(struct net_device *dev, struct ifreq *rq)
{
	struct hwtstamp_config *tstamp_config;
	struct phytmac *pdata = netdev_priv(dev);

	if (!IS_REACHABLE(CONFIG_PHYTMAC_ENABLE_PTP))
		return -EOPNOTSUPP;

	tstamp_config = &pdata->ts_config;

	if (copy_to_user(rq->ifr_data, tstamp_config, sizeof(*tstamp_config)))
		return -EFAULT;
	else
		return 0;
}
int phytmac_ptp_set_ts_config(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct hwtstamp_config config;
	struct phytmac *pdata = netdev_priv(dev);
	struct phytmac_hw_if *hw_if = pdata->hw_if;
	struct ts_ctrl tstamp_ctrl;
	int ret;

	memset(&tstamp_ctrl, 0, sizeof(struct ts_ctrl));

	if (copy_from_user(&config, ifr->ifr_data, sizeof(config)))
		return -EFAULT;

	switch (config.tx_type) {
	case HWTSTAMP_TX_OFF:
		break;
	case HWTSTAMP_TX_ONESTEP_SYNC:
		tstamp_ctrl.one_step = 1;
		tstamp_ctrl.tx_control = TS_ALL_FRAMES;
		break;
	case HWTSTAMP_TX_ON:
		tstamp_ctrl.one_step = 0;
		tstamp_ctrl.tx_control = TS_ALL_FRAMES;
		break;
	default:
		return -ERANGE;
	}

	switch (config.rx_filter) {
	case HWTSTAMP_FILTER_NONE:
	case HWTSTAMP_FILTER_PTP_V1_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ:
		break;
	case HWTSTAMP_FILTER_PTP_V2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_L4_EVENT:
	case HWTSTAMP_FILTER_PTP_V2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L4_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
	case HWTSTAMP_FILTER_PTP_V2_L4_DELAY_REQ:
		tstamp_ctrl.rx_control = TS_ALL_PTP_FRAMES;
		config.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
		break;
	case HWTSTAMP_FILTER_PTP_V1_L4_EVENT:
	case HWTSTAMP_FILTER_ALL:
		tstamp_ctrl.rx_control = TS_ALL_FRAMES;
		config.rx_filter = HWTSTAMP_FILTER_ALL;
		break;
	default:
		config.rx_filter = HWTSTAMP_FILTER_NONE;
		return -ERANGE;
	}

	ret = hw_if->set_ts_config(pdata, &tstamp_ctrl);
	if (ret)
		return ret;

	/* save these settings for future reference */
	pdata->ts_config = config;
	memcpy(&pdata->ts_config, &config, sizeof(config));

	if (copy_to_user(ifr->ifr_data, &config, sizeof(config)))
		return -EFAULT;
	else
		return 0;
}
