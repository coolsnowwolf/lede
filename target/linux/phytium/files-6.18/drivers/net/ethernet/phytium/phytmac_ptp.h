/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Phytium Ethernet Controller driver
 *
 * Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PHYTMAC_PTP_H
#define _PHYTMAC_PTP_H

#ifdef CONFIG_PHYTMAC_ENABLE_PTP
bool phytmac_ptp_one_step(struct sk_buff *skb);
int phytmac_ptp_gettime(struct ptp_clock_info *ptp, struct timespec64 *ts);
int phytmac_ptp_settime(struct ptp_clock_info *ptp,
			const struct timespec64 *ts);
int phytmac_ptp_adjfine(struct ptp_clock_info *ptp, long scaled_ppm);
int phytmac_ptp_adjtime(struct ptp_clock_info *ptp, s64 delta);
int phytmac_ptp_enable(struct ptp_clock_info *ptp,
		       struct ptp_clock_request *rq, int on);
void phytmac_ptp_init_timer(struct phytmac *pdata);
void phytmac_ptp_rxstamp(struct phytmac *pdata, struct sk_buff *skb,
			 struct phytmac_dma_desc *desc);
int phytmac_ptp_txstamp(struct phytmac_queue *queue, struct sk_buff *skb,
			struct phytmac_dma_desc *desc);
int phytmac_ptp_register(struct phytmac *pdata);
void phytmac_ptp_unregister(struct phytmac *pdata);
void phytmac_ptp_init(struct net_device *ndev);
int phytmac_ptp_get_ts_config(struct net_device *dev, struct ifreq *rq);
int phytmac_ptp_set_ts_config(struct net_device *dev, struct ifreq *ifr, int cmd);
#else
static inline bool phytmac_ptp_one_step(struct sk_buff *skb)
{
	return 1;
}

static inline void phytmac_ptp_rxstamp(struct phytmac *pdata, struct sk_buff *skb,
				       struct phytmac_dma_desc *desc) {}
static inline int phytmac_ptp_txstamp(struct phytmac_queue *queue, struct sk_buff *skb,
				      struct phytmac_dma_desc *desc)
{
	return -1;
}

static inline int phytmac_ptp_register(struct phytmac *pdata)
{
	return 0;
}

static inline void phytmac_ptp_unregister(struct phytmac *pdata) {}
static inline void phytmac_ptp_init(struct net_device *ndev) {}

#endif
#endif
