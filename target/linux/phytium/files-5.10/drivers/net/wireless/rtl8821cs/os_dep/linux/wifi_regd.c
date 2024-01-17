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

#include <drv_types.h>

#ifdef CONFIG_IOCTL_CFG80211
void rtw_chset_hook_os_channels(struct rf_ctl_t *rfctl)
{
	struct wiphy *wiphy = dvobj_to_wiphy(rfctl_to_dvobj(rfctl));
	RT_CHANNEL_INFO *channel_set = rfctl->channel_set;
	u8 max_chan_nums = rfctl->max_chan_nums;
	struct ieee80211_channel *ch;
	unsigned int i;
	u16 channel;
	u32 freq;

	for (i = 0; i < max_chan_nums; i++) {
		channel = channel_set[i].ChannelNum;
		#if CONFIG_IEEE80211_BAND_6GHZ
		if (channel_set[i].band == BAND_ON_6G)
			continue; /* TODO: wiphy with 6G band */
		else
		#endif
			freq = rtw_ch2freq(channel);
		ch = ieee80211_get_channel(wiphy, freq);
		if (!ch) {
			rtw_warn_on(1);
			continue;
		}

		channel_set[i].os_chan = ch;
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
bool rtw_regd_is_wiphy_self_managed(struct wiphy *wiphy)
{
	return rtw_rfctl_is_disable_sw_channel_plan(wiphy_to_dvobj(wiphy))
		|| !REGSTY_REGD_SRC_FROM_OS(dvobj_to_regsty(wiphy_to_dvobj(wiphy)));
}

static struct ieee80211_reg_rule rtw_get_ieee80211_reg_rule(struct get_chplan_resp *chplan, BAND_TYPE band, int last_end_freq, int start_freq, int end_freq, int next_start_freq, u32 flags)
{
	struct ieee80211_reg_rule rule = REG_RULE(
		start_freq - 10, end_freq + 10, 20, 6, 20,
		((flags & RTW_CHF_NO_IR) ? NL80211_RRF_NO_IR : 0)
		//| ((flags & RTW_CHF_DFS) ? NL80211_RRF_DFS : 0) /* TODO: DFS */
	);

	int regd_max_bw = 160;
	int frange_max_bw = 160;

	if (!(chplan->proto_en & CHPLAN_PROTO_EN_AC) || band == BAND_ON_2_4G)
		regd_max_bw = 40;
	/* TODO: !RFCTL_REG_EN_11HT(rfctl) limit to 20MHz  */

	while ((end_freq - start_freq + 20) < frange_max_bw) {
		frange_max_bw /= 2;
		if (frange_max_bw == 20)
			break;
	}
	rule.freq_range.max_bandwidth_khz = MHZ_TO_KHZ(rtw_min(regd_max_bw, frange_max_bw));

	if (regd_max_bw > frange_max_bw
		&& (rtw_freq_consecutive(last_end_freq, start_freq)
			|| rtw_freq_consecutive(end_freq, next_start_freq)
		)
	)
		rule.flags |= NL80211_RRF_AUTO_BW;

	if (regd_max_bw < 40)
		rule.flags |= NL80211_RRF_NO_HT40;
	if (regd_max_bw < 80)
		rule.flags |= NL80211_RRF_NO_80MHZ;
	if (regd_max_bw < 160)
		rule.flags |= NL80211_RRF_NO_160MHZ;

	return rule;
}

static int rtw_build_wiphy_regd(struct wiphy *wiphy, struct get_chplan_resp *chplan, struct ieee80211_regdomain **regd)
{
	int i;
	RT_CHANNEL_INFO *chinfo;
	BAND_TYPE start_band, band;
	int last_end_freq, start_freq, end_freq, freq;
	u32 start_flags, flags;
	struct ieee80211_regdomain *r;
	int rule_num = 0;
	bool build = 0;

	if (regd)
		*regd = NULL;

loop:
	start_band = BAND_MAX;
	last_end_freq = 0;
	for (i = 0; i < chplan->chset_num; i++) {
		chinfo = &chplan->chset[i];
		freq = rtw_ch2freq_by_band(chinfo->band, chinfo->ChannelNum);
		if (!freq) {
			RTW_WARN(FUNC_WIPHY_FMT" rtw_ch2freq_by_band(%s, %u) fail\n"
				, FUNC_WIPHY_ARG(wiphy), band_str(chinfo->band), chinfo->ChannelNum);
			continue;
		}
		band = chinfo->band;
		flags = chinfo->flags & (RTW_CHF_NO_IR | RTW_CHF_DFS);

		if (start_band == BAND_MAX) {
			start_band = band;
			start_freq = end_freq = freq;
			start_flags = flags;
			continue;
		}

		if (start_band == band
			&& start_flags == flags
			&& rtw_freq_consecutive(end_freq, freq)
		) {
			end_freq = freq;
			continue;
		}

		/* create rule */
		if (build) {
			RTW_DBG("add rule_%02d(%s, %d, %d, 0x%x)\n"
				, r->n_reg_rules, band_str(start_band), start_freq, end_freq, start_flags);
			r->reg_rules[r->n_reg_rules++] = rtw_get_ieee80211_reg_rule(chplan, start_band
				, last_end_freq, start_freq, end_freq, freq, start_flags);
		} else
			rule_num++;

		/* start a new rule */
		start_band = band;
		last_end_freq = end_freq;
		start_freq = end_freq = freq;
		start_flags = flags;
	}

	if (start_band != BAND_MAX) {
		/* create rule */
		if (build) {
			RTW_DBG("add rule_%02d(%s, %d, %d, 0x%x)\n"
				, r->n_reg_rules, band_str(start_band), start_freq, end_freq, start_flags);
			r->reg_rules[r->n_reg_rules++] = rtw_get_ieee80211_reg_rule(chplan, start_band
				, last_end_freq, start_freq, end_freq, 0, start_flags);
		} else
			rule_num++;
	}

	if (!build) {
		/* switch to build phase */
		build = 1;
		if (!regd)
			goto exit;

		r = rtw_zmalloc(sizeof(**regd) + sizeof(struct ieee80211_reg_rule) * rule_num);
		if (!r) {
			rule_num = -1;
			goto exit;
		}

		_rtw_memcpy(r->alpha2, chplan->alpha2, 2);
		r->alpha2[2] = 0;
		r->dfs_region = NL80211_DFS_UNSET;
		goto loop;
	}

	*regd = r;

exit:
	return rule_num;
}

static void rtw_regd_disable_no_20mhz_chs(struct wiphy *wiphy)
{
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	unsigned int i, j;

	for (i = 0; i < NUM_NL80211_BANDS; i++) {
		sband = wiphy->bands[i];
		if (!sband)
			continue;
		for (j = 0; j < sband->n_channels; j++) {
			ch = &sband->channels[j];
			if (!ch)
				continue;
			if (ch->flags & IEEE80211_CHAN_NO_20MHZ) {
				RTW_INFO(FUNC_WIPHY_FMT" disable band:%d ch:%u w/o 20MHz\n", FUNC_WIPHY_ARG(wiphy), ch->band, ch->hw_value);
				ch->flags = IEEE80211_CHAN_DISABLED;
			}
		}
	}
}

void rtw_update_wiphy_regd(struct wiphy *wiphy, struct get_chplan_resp *chplan, bool rtnl_lock_needed)
{
	struct ieee80211_regdomain *regd;
	int ret;

	ret = rtw_build_wiphy_regd(wiphy, chplan, &regd);
	if (ret == -1) {
		RTW_WARN(FUNC_WIPHY_FMT" rtw_build_wiphy_regd() fail\n", FUNC_WIPHY_ARG(wiphy));
		return;
	}

	if (ret == 0) {
		RTW_WARN(FUNC_WIPHY_FMT" rtw_build_wiphy_regd() builds empty regd, bypass regd setting\n", FUNC_WIPHY_ARG(wiphy));
		goto free_regd;
	}

	if (rtnl_lock_needed)
		rtnl_lock();

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
	ret = regulatory_set_wiphy_regd_sync(wiphy, regd);
	#else
	ret = regulatory_set_wiphy_regd_sync_rtnl(wiphy, regd);
	#endif

	rtw_regd_disable_no_20mhz_chs(wiphy);

	if (rtnl_lock_needed)
		rtnl_unlock();

	if (ret != 0)
		RTW_INFO(FUNC_WIPHY_FMT" regulatory_set_wiphy_regd_sync_rtnl return %d\n", FUNC_WIPHY_ARG(wiphy), ret);

free_regd:
	rtw_mfree(regd, sizeof(*regd) + sizeof(struct ieee80211_reg_rule) * regd->n_reg_rules);
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) */

static void rtw_regd_overide_flags(struct wiphy *wiphy, struct get_chplan_resp *chplan, bool rtnl_lock_needed)
{
	RT_CHANNEL_INFO *channel_set = chplan->chset;
	u8 max_chan_nums = chplan->chset_num;
	struct ieee80211_supported_band *sband;
	struct ieee80211_channel *ch;
	unsigned int i, j;

	if (rtnl_lock_needed)
		rtnl_lock();

	/* all channels disable */
	for (i = 0; i < NUM_NL80211_BANDS; i++) {
		sband = wiphy->bands[i];
		if (!sband)
			continue;
		for (j = 0; j < sband->n_channels; j++) {
			ch = &sband->channels[j];
			if (!ch)
				continue;
			ch->flags = IEEE80211_CHAN_DISABLED;
		}
	}

	/* channels apply by channel plans. */
	for (i = 0; i < max_chan_nums; i++) {
		ch = channel_set[i].os_chan;
		if (!ch)
			continue;

		/* enable */
		ch->flags = 0;

		if (channel_set[i].flags & RTW_CHF_DFS) {
			/*
			* before integrating with nl80211 flow
			* bypass IEEE80211_CHAN_RADAR when configured with radar detection
			* to prevent from hostapd blocking DFS channels
			*/
			#ifdef CONFIG_DFS_MASTER
			if (chplan->dfs_domain == RTW_DFS_REGD_NONE)
			#endif
				ch->flags |= IEEE80211_CHAN_RADAR;
		}

		if (channel_set[i].flags & RTW_CHF_NO_IR) {
			#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
			ch->flags |= IEEE80211_CHAN_NO_IBSS | IEEE80211_CHAN_PASSIVE_SCAN;
			#else
			ch->flags |= IEEE80211_CHAN_NO_IR;
			#endif
		}
	}

	if (rtnl_lock_needed)
		rtnl_unlock();
}

#ifdef CONFIG_REGD_SRC_FROM_OS
static void rtw_regd_apply_dfs_flags(struct get_chplan_resp *chplan, bool rtnl_lock_needed)
{
	RT_CHANNEL_INFO *channel_set = chplan->chset;
	u8 max_chan_nums = chplan->chset_num;
	unsigned int i;
	struct ieee80211_channel *chan;

	if (rtnl_lock_needed)
		rtnl_lock();

	/* channels apply by channel plans. */
	for (i = 0; i < max_chan_nums; i++) {
		chan = channel_set[i].os_chan;
		if (!chan) {
			rtw_warn_on(1);
			continue;
		}
		if (channel_set[i].flags & RTW_CHF_DFS) {
			/*
			* before integrating with nl80211 flow
			* clear IEEE80211_CHAN_RADAR when configured with radar detection
			* to prevent from hostapd blocking DFS channels
			*/
			#ifdef CONFIG_DFS_MASTER
			if (chplan->dfs_domain != RTW_DFS_REGD_NONE)
				chan->flags &= ~IEEE80211_CHAN_RADAR;
			#endif
		}
	}

	if (rtnl_lock_needed)
		rtnl_unlock();
}

/* init_channel_set_from_wiphy */
u8 rtw_os_init_channel_set(_adapter *padapter, RT_CHANNEL_INFO *channel_set)
{
	struct wiphy *wiphy = adapter_to_wiphy(padapter);
	struct rf_ctl_t *rfctl = adapter_to_rfctl(padapter);
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	struct ieee80211_channel *chan;
	u8 chanset_size = 0;
	int i, j;

	_rtw_memset(channel_set, 0, sizeof(RT_CHANNEL_INFO) * MAX_CHANNEL_NUM);

	for (i = NL80211_BAND_2GHZ; i <= NL80211_BAND_5GHZ; i++) {
		if (!wiphy->bands[i])
			continue;
		for (j = 0; j < wiphy->bands[i]->n_channels; j++) {
			chan = &wiphy->bands[i]->channels[j];
			if (chan->flags & IEEE80211_CHAN_DISABLED)
				continue;
			if (rtw_regsty_is_excl_chs(regsty, chan->hw_value))
				continue;

			if (chanset_size >= MAX_CHANNEL_NUM) {
				RTW_WARN("chset size can't exceed MAX_CHANNEL_NUM(%u)\n", MAX_CHANNEL_NUM);
				i = NL80211_BAND_5GHZ + 1;
				break;
			}

			channel_set[chanset_size].ChannelNum = chan->hw_value;
			channel_set[chanset_size].band = nl80211_band_to_rtw_band(i);
			#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
			if (chan->flags & (IEEE80211_CHAN_NO_IBSS | IEEE80211_CHAN_PASSIVE_SCAN))
			#else
			if (chan->flags & IEEE80211_CHAN_NO_IR)
			#endif
				channel_set[chanset_size].flags |= RTW_CHF_NO_IR;
			if (chan->flags & IEEE80211_CHAN_RADAR)
				channel_set[chanset_size].flags |= RTW_CHF_DFS;
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
			if (chan->flags & IEEE80211_CHAN_NO_HT40PLUS)
				channel_set[chanset_size].flags |= RTW_CHF_NO_HT40U;
			if (chan->flags & IEEE80211_CHAN_NO_HT40MINUS)
				channel_set[chanset_size].flags |= RTW_CHF_NO_HT40L;
			#endif
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
			if (chan->flags & IEEE80211_CHAN_NO_80MHZ)
				channel_set[chanset_size].flags |= RTW_CHF_NO_80MHZ;
			if (chan->flags & IEEE80211_CHAN_NO_160MHZ)
				channel_set[chanset_size].flags |= RTW_CHF_NO_160MHZ;
			#endif
			channel_set[chanset_size].os_chan = chan;
			chanset_size++;
		}
	}

#if CONFIG_IEEE80211_BAND_5GHZ
	#ifdef CONFIG_DFS_MASTER
	for (i = 0; i < chanset_size; i++)
		channel_set[i].non_ocp_end_time = rtw_get_current_time();
	#endif
#endif /* CONFIG_IEEE80211_BAND_5GHZ */

	if (chanset_size)
		RTW_INFO(FUNC_ADPT_FMT" ch num:%d\n"
			, FUNC_ADPT_ARG(padapter), chanset_size);
	else
		RTW_WARN(FUNC_ADPT_FMT" final chset has no channel\n"
			, FUNC_ADPT_ARG(padapter));

	return chanset_size;
}

s16 rtw_os_get_total_txpwr_regd_lmt_mbm(_adapter *adapter, u8 cch, enum channel_width bw)
{
	struct wiphy *wiphy = adapter_to_wiphy(adapter);
	s16 mbm = UNSPECIFIED_MBM;
	u8 *op_chs;
	u8 op_ch_num;
	u8 i;
	u32 freq;
	struct ieee80211_channel *ch;

	if (!rtw_get_op_chs_by_cch_bw(cch, bw, &op_chs, &op_ch_num))
		goto exit;

	for (i = 0; i < op_ch_num; i++) {
		freq = rtw_ch2freq(op_chs[i]);
		ch = ieee80211_get_channel(wiphy, freq);
		if (!ch) {
			rtw_warn_on(1);
			continue;
		}
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		mbm = rtw_min(mbm, ch->max_reg_power * MBM_PDBM);
		#else
		/* require max_power == 0 (therefore orig_mpwr set to 0) when wiphy registration */
		mbm = rtw_min(mbm, ch->max_power * MBM_PDBM);
		#endif
	}

exit:
	return mbm;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
static enum rtw_dfs_regd nl80211_dfs_regions_to_rtw_dfs_region(enum nl80211_dfs_regions region)
{
	switch (region) {
	case NL80211_DFS_FCC:
		return RTW_DFS_REGD_FCC;
	case NL80211_DFS_ETSI:
		return RTW_DFS_REGD_ETSI;
	case NL80211_DFS_JP:
		return RTW_DFS_REGD_MKK;
	case NL80211_DFS_UNSET:
	default:
		return RTW_DFS_REGD_NONE;
	}
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)) */
#endif /* CONFIG_REGD_SRC_FROM_OS */

static enum rtw_regd_inr nl80211_reg_initiator_to_rtw_regd_inr(enum nl80211_reg_initiator initiator)
{
	switch (initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
		return RTW_REGD_SET_BY_DRIVER;
	case NL80211_REGDOM_SET_BY_CORE:
		return RTW_REGD_SET_BY_CORE;
	case NL80211_REGDOM_SET_BY_USER:
		return RTW_REGD_SET_BY_USER;
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		return RTW_REGD_SET_BY_COUNTRY_IE;
	}
	rtw_warn_on(1);
	return RTW_REGD_SET_BY_NUM;
};

#ifdef CONFIG_RTW_DEBUG
static const char *nl80211_reg_initiator_str(enum nl80211_reg_initiator initiator)
{
	switch (initiator) {
	case NL80211_REGDOM_SET_BY_DRIVER:
		return "DRIVER";
	case NL80211_REGDOM_SET_BY_CORE:
		return "CORE";
	case NL80211_REGDOM_SET_BY_USER:
		return "USER";
	case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		return "COUNTRY_IE";
	}
	rtw_warn_on(1);
	return "UNKNOWN";
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
static const char *nl80211_user_reg_hint_type_str(enum nl80211_user_reg_hint_type type)
{
	switch (type) {
	case NL80211_USER_REG_HINT_USER:
		return "USER";
	case NL80211_USER_REG_HINT_CELL_BASE:
		return "CELL_BASE";
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	case NL80211_USER_REG_HINT_INDOOR:
		return "INDOOR";
	#endif
	}
	rtw_warn_on(1);
	return "UNKNOWN";
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
static const char *nl80211_dfs_regions_str(enum nl80211_dfs_regions region)
{
	switch (region) {
	case NL80211_DFS_UNSET:
		return "UNSET";
	case NL80211_DFS_FCC:
		return "FCC";
	case NL80211_DFS_ETSI:
		return "ETSI";
	case NL80211_DFS_JP:
		return "JP";
	}
	rtw_warn_on(1);
	return "UNKNOWN";
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)) */

static const char *environment_cap_str(enum environment_cap cap)
{
	switch (cap) {
	case ENVIRON_ANY:
		return "ANY";
	case ENVIRON_INDOOR:
		return "INDOOR";
	case ENVIRON_OUTDOOR:
		return "OUTDOOR";
	}
	rtw_warn_on(1);
	return "UNKNOWN";
}

static void dump_requlatory_request(void *sel, struct regulatory_request *request)
{
	u8 alpha2_len;

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0))
	alpha2_len = 3;
	#else
	alpha2_len = 2;
	#endif

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	RTW_PRINT_SEL(sel, "initiator:%s, wiphy_idx:%d, type:%s\n"
		, nl80211_reg_initiator_str(request->initiator)
		, request->wiphy_idx
		, nl80211_user_reg_hint_type_str(request->user_reg_hint_type));
	#else
	RTW_PRINT_SEL(sel, "initiator:%s, wiphy_idx:%d\n"
		, nl80211_reg_initiator_str(request->initiator)
		, request->wiphy_idx);
	#endif

	RTW_PRINT_SEL(sel, "alpha2:%.*s\n", alpha2_len, request->alpha2);
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
	RTW_PRINT_SEL(sel, "dfs_region:%s\n", nl80211_dfs_regions_str(request->dfs_region));
	#endif

	RTW_PRINT_SEL(sel, "intersect:%d\n", request->intersect);
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38))
	RTW_PRINT_SEL(sel, "processed:%d\n", request->processed);
	#endif
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
	RTW_PRINT_SEL(sel, "country_ie_checksum:0x%08x\n", request->country_ie_checksum);
	#endif

	RTW_PRINT_SEL(sel, "country_ie_env:%s\n", environment_cap_str(request->country_ie_env));
}
#endif /* CONFIG_RTW_DEBUG */

static void rtw_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request)
{
	struct dvobj_priv *dvobj = wiphy_to_dvobj(wiphy);
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);
	enum rtw_regd_inr inr;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	rtw_set_rtnl_lock_holder(dvobj, current);
#endif

#ifdef CONFIG_RTW_DEBUG
	if (rtw_drv_log_level >= _DRV_INFO_) {
		RTW_INFO(FUNC_WIPHY_FMT"\n", FUNC_WIPHY_ARG(wiphy));
		dump_requlatory_request(RTW_DBGDUMP, request);
	}
#endif

	inr = nl80211_reg_initiator_to_rtw_regd_inr(request->initiator);

#ifdef CONFIG_REGD_SRC_FROM_OS
	if (REGSTY_REGD_SRC_FROM_OS(regsty)) {
		enum rtw_dfs_regd dfs_region =  RTW_DFS_REGD_NONE;

		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
		dfs_region = nl80211_dfs_regions_to_rtw_dfs_region(request->dfs_region);
		#endif

		/* trigger command to sync regulatory form OS */
		rtw_sync_os_regd_cmd(wiphy_to_adapter(wiphy), RTW_CMDF_WAIT_ACK, request->alpha2, dfs_region, inr);
	} else
#endif
	{
		/* use alpha2 as input to select the corresponding channel plan settings defined by Realtek */
		struct get_chplan_resp *chplan;

		switch (request->initiator) {
		case NL80211_REGDOM_SET_BY_USER:
			rtw_set_country(wiphy_to_adapter(wiphy), request->alpha2, inr);
			break;
		case NL80211_REGDOM_SET_BY_DRIVER:
		case NL80211_REGDOM_SET_BY_CORE:
		case NL80211_REGDOM_SET_BY_COUNTRY_IE:
		default:
			#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
			rtw_warn_on(rtw_regd_is_wiphy_self_managed(wiphy));
			#endif
			if (rtw_get_chplan_cmd(wiphy_to_adapter(wiphy), RTW_CMDF_WAIT_ACK, &chplan) == _SUCCESS)
				rtw_regd_change_complete_sync(wiphy, chplan, 0);
			else
				rtw_warn_on(1);
			break;
		}
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	rtw_set_rtnl_lock_holder(dvobj, NULL);
#endif
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
static int rtw_reg_notifier_return(struct wiphy *wiphy, struct regulatory_request *request)
{
	rtw_reg_notifier(wiphy, request);
	return 0;
}
#endif

struct async_regd_change_evt {
	_list list; /* async_regd_change_list */
	struct wiphy *wiphy;
	struct get_chplan_resp *chplan;
};

static void async_regd_change_work_hdl(_workitem *work)
{
	struct rtw_wiphy_data *wiphy_data = container_of(work, struct rtw_wiphy_data, async_regd_change_work);
	struct async_regd_change_evt *evt;
	_irqL irqL;
	_list *list, *head = &wiphy_data->async_regd_change_list;

	while (1) {
		_enter_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);
		list = rtw_is_list_empty(head) ? NULL : get_next(head);
		if (list)
			rtw_list_delete(list);
		_exit_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);

		if (!list)
			break;

		evt = LIST_CONTAINOR(list, struct async_regd_change_evt, list);
		rtw_regd_change_complete_sync(evt->wiphy, evt->chplan, 1);
		rtw_mfree(evt, sizeof(*evt));
	}
}

int rtw_regd_init(struct wiphy *wiphy)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0))
	wiphy->reg_notifier = rtw_reg_notifier_return;
#else
	wiphy->reg_notifier = rtw_reg_notifier;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
	wiphy->flags &= ~WIPHY_FLAG_STRICT_REGULATORY;
	wiphy->flags &= ~WIPHY_FLAG_DISABLE_BEACON_HINTS;
#else
	wiphy->regulatory_flags &= ~REGULATORY_STRICT_REG;
	wiphy->regulatory_flags &= ~REGULATORY_DISABLE_BEACON_HINTS;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	wiphy->regulatory_flags |= REGULATORY_IGNORE_STALE_KICKOFF;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	if (rtw_regd_is_wiphy_self_managed(wiphy))
		wiphy->regulatory_flags |= REGULATORY_WIPHY_SELF_MANAGED;
#endif

	_rtw_init_listhead(&wiphy_data->async_regd_change_list);
	_rtw_mutex_init(&wiphy_data->async_regd_change_mutex);
	_init_workitem(&wiphy_data->async_regd_change_work, async_regd_change_work_hdl, NULL);

	return 0;
}

static void rtw_regd_async_regd_change_list_free(struct wiphy *wiphy)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);
	struct async_regd_change_evt *evt;
	struct get_chplan_resp *chplan;
	_irqL irqL;
	_list *cur, *head;

	_enter_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);

	head = &wiphy_data->async_regd_change_list;
	cur = get_next(head);

	while ((rtw_end_of_queue_search(head, cur)) == _FALSE) {
		evt = LIST_CONTAINOR(cur, struct async_regd_change_evt, list);
		chplan = evt->chplan;
		cur = get_next(cur);
		rtw_list_delete(&evt->list);
		rtw_vmfree(chplan, sizeof(*chplan) + sizeof(RT_CHANNEL_INFO) * chplan->chset_num);
		rtw_mfree(evt, sizeof(*evt));
	}

	_exit_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);
}

void rtw_regd_deinit(struct wiphy *wiphy)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);

	_cancel_workitem_sync(&wiphy_data->async_regd_change_work);
	rtw_regd_async_regd_change_list_free(wiphy);
	_rtw_mutex_free(&wiphy_data->async_regd_change_mutex);
}

void rtw_regd_change_complete_sync(struct wiphy *wiphy, struct get_chplan_resp *chplan, bool rtnl_lock_needed)
{
	if (chplan->regd_src == REGD_SRC_RTK_PRIV) {
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		if (rtw_regd_is_wiphy_self_managed(wiphy))
			rtw_update_wiphy_regd(wiphy, chplan, rtnl_lock_needed);
		else
		#endif
			rtw_regd_overide_flags(wiphy, chplan, rtnl_lock_needed);
	}
#ifdef CONFIG_REGD_SRC_FROM_OS
	else if (chplan->regd_src == REGD_SRC_OS)
		rtw_regd_apply_dfs_flags(chplan, rtnl_lock_needed);
#endif
	else
		rtw_warn_on(1);

	rtw_vmfree(chplan, sizeof(struct get_chplan_resp) + sizeof(RT_CHANNEL_INFO) * chplan->chset_num);
}

int rtw_regd_change_complete_async(struct wiphy *wiphy, struct get_chplan_resp *chplan)
{
	struct rtw_wiphy_data *wiphy_data = rtw_wiphy_priv(wiphy);
	struct async_regd_change_evt *evt;
	_irqL irqL;

	evt = rtw_malloc(sizeof(*evt));
	if (!evt) {
		rtw_vmfree(chplan, sizeof(struct get_chplan_resp) + sizeof(RT_CHANNEL_INFO) * chplan->chset_num);
		return _FAIL;
	}

	_rtw_init_listhead(&evt->list);
	evt->wiphy = wiphy;
	evt->chplan = chplan;

	_enter_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);

	rtw_list_insert_tail(&evt->list, &wiphy_data->async_regd_change_list);

	_exit_critical_mutex(&wiphy_data->async_regd_change_mutex, &irqL);

	_set_workitem(&wiphy_data->async_regd_change_work);

	return _SUCCESS;
}
#endif /* CONFIG_IOCTL_CFG80211 */
