/******************************************************************************
 *
 * Copyright(c) 2009-2010 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#ifndef __WIFI_REGD_H__
#define __WIFI_REGD_H__

void rtw_chset_hook_os_channels(struct rf_ctl_t *rfctl);
void rtw_regd_change_complete_sync(struct wiphy *wiphy, struct get_chplan_resp *chplan, bool rtnl_lock_needed);
int rtw_regd_change_complete_async(struct wiphy *wiphy, struct get_chplan_resp *chplan);
#ifdef CONFIG_REGD_SRC_FROM_OS
struct _RT_CHANNEL_INFO;
u8 rtw_os_init_channel_set(_adapter *padapter, struct _RT_CHANNEL_INFO *channel_set);
s16 rtw_os_get_total_txpwr_regd_lmt_mbm(_adapter *adapter, u8 cch, enum channel_width bw);
#endif
int rtw_regd_init(struct wiphy *wiphy);
void rtw_regd_deinit(struct wiphy *wiphy);

#endif /* __WIFI_REGD_H__ */
