// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#include <linux/version.h>
#include <linux/iopoll.h>
#include <drm/drm_atomic_helper.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
#include <drm/display/drm_dp.h>
#else
#include <drm/drm_dp_helper.h>
#endif
#include "phytium_dp.h"
#include "phytium_dp_reg.h"
#include "phytium_psr.h"

static ssize_t phytium_dp_get_source_psr_status(struct phytium_dp_device *phytium_dp)
{
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	ssize_t status;

	status = phytium_readl_reg(priv, group_offset, SEC_PSR_STATE_INTERNAL);
	if (status < 0) {
		DRM_ERROR("PSR_STATUS read failed ret=%zd", status);
		return status;
	}
	return status;
}

static ssize_t phytium_dp_get_sink_psr_status(struct phytium_dp_device *phytium_dp)
{
	ssize_t val;
	uint8_t status;

	val = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_STATUS, &status);
	//val = drm_dp_dpcd_readb(&phytium_dp->aux, DP_LAST_RECEIVED_PSR_SDP, &status);
	if (val < 0) {
		DRM_ERROR("PSR_STATUS read failed ret=%zd", val);
		return val;
	}
	return status;
}

static bool phytium_dp_get_alpm_status(struct phytium_dp_device *phytium_dp)
{
	uint8_t alpm_caps = 0;

	if (drm_dp_dpcd_readb(&phytium_dp->aux, DP_RECEIVER_ALPM_CAP,
			      &alpm_caps) != 1)
		return false;
	return alpm_caps & DP_ALPM_CAP;
}

/*
 * Check if sink supports the pixel encoding format and chroma format
 * indicated by source (by setting the MISC1 field)
 */
static bool phytium_dp_get_colorimetry_status(struct phytium_dp_device *phytium_dp)
{
	uint8_t dprx = 0;

	if (drm_dp_dpcd_readb(&phytium_dp->aux, DP_DPRX_FEATURE_ENUMERATION_LIST,
			      &dprx) != 1)
		return false;
	return dprx & DP_VSC_SDP_EXT_FOR_COLORIMETRY_SUPPORTED;
}

/*
 * Whether the coordinates of an optional range update
 * need to be updated according to specific granularity requirements,
 * not supported by kernel 4.19
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
static void phytium_dp_get_su_granularity(struct phytium_dp_device *phytium_dp)
{
	ssize_t r;
	uint16_t w;
	uint8_t y;

	/* If sink don't have specific granularity requirements set legacy ones */
	if (!(phytium_dp->psr_dpcd[1] & DP_PSR2_SU_GRANULARITY_REQUIRED)) {
		/* As PSR2 HW sends full lines, we do not care about x granularity */
		w = 4;
		y = 4;
		goto out;
	}

	r = drm_dp_dpcd_read(&phytium_dp->aux, DP_PSR2_SU_X_GRANULARITY, &w, 2);
	if (r != 2)
		DRM_DEBUG_KMS("Unable to read DP_PSR2_SU_X_GRANULARITY\n");
	/*
	 * Spec says that if the value read is 0 the default granularity should
	 * be used instead.
	 */
	if (r != 2 || w == 0)
		w = 4;

	r = drm_dp_dpcd_read(&phytium_dp->aux, DP_PSR2_SU_Y_GRANULARITY, &y, 1);
	if (r != 1) {
		DRM_DEBUG_KMS("Unable to read DP_PSR2_SU_Y_GRANULARITY\n");
		y = 4;
	}
	if (y == 0)
		y = 1;

out:
	phytium_dp->psr.su_w_granularity = w;
	phytium_dp->psr.su_y_granularity = y;
}
#endif
void phytium_psr_init_dpcd(struct phytium_dp_device *phytium_dp)
{
	int ret;
	uint8_t val;

	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_PSR_SUPPORT,
							phytium_dp->psr_dpcd,
							sizeof(phytium_dp->psr_dpcd));
	if (ret < 0) {
		DRM_ERROR("port %d get PSR capability fail\n", phytium_dp->port);
		return;
	}

	if (!(phytium_dp->edp_dpcd[1] & DP_EDP_SET_POWER_CAP)) {
		DRM_DEBUG_KMS("Panel lacks power state control, PSR cannot be enabled\n");
		return;
	}

	if (!phytium_dp->psr_dpcd[0]) {
		phytium_dp->psr.sink_support = false;
		return;
	}

	DRM_DEBUG_KMS("eDP panel supports PSR version %x\n", phytium_dp->psr_dpcd[0]);
	phytium_dp->psr.sink_support = true;

	/* Maximum number of frames needed by the Sink for synchronizing to Source timing */
	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_SYNCHRONIZATION_LATENCY_IN_SINK, &val);
	if (ret < 0) {
		DRM_DEBUG_KMS("port %d get PSR SYNC latency fail\n", phytium_dp->port);
		val = 8;
	} else
		val &= DP_MAX_RESYNC_FRAME_COUNT_MASK;

	phytium_dp->psr.sink_sync_latency = val;

#if PSR_DEBUG
	pr_info("Max re-sync frame count needed by sink is %d\n", val);
#endif

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_CAPS, &val);
	if (ret < 0)
		DRM_DEBUG_KMS("port %d get PSR CAPS fail\n", phytium_dp->port);

	phytium_dp->psr.sink_need_train_on_exit = val & DP_PSR_NO_TRAIN_ON_EXIT;
	phytium_dp->psr.sink_setup_time = drm_dp_psr_setup_time(phytium_dp->psr_dpcd);
#if PSR_DEBUG
	pr_info("sink %sneed train on exit\n",
			phytium_dp->psr.sink_need_train_on_exit ? "" : "does not ");
#endif
	/* If sink supports psr2 */
	if (phytium_dp->psr_dpcd[0] == DP_PSR2_WITH_Y_COORD_IS_SUPPORTED) {
		bool y_req = phytium_dp->psr_dpcd[1] &
			     DP_PSR2_SU_Y_COORDINATE_REQUIRED;
		bool alpm = phytium_dp_get_alpm_status(phytium_dp);

		phytium_dp->psr.sink_psr2_support = y_req && alpm;
		DRM_DEBUG_KMS("PSR2 %ssupported\n",
						phytium_dp->psr.sink_psr2_support ? "" : "not ");
		if (phytium_dp->psr.sink_psr2_support) {
			phytium_dp->psr.colorimetry_support =
				phytium_dp_get_colorimetry_status(phytium_dp);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
			phytium_dp_get_su_granularity(phytium_dp);
#endif
		}
	}

	phytium_dp->psr.psr_setup_time_us = drm_dp_psr_setup_time(phytium_dp->psr_dpcd);
	if (phytium_dp->psr.psr_setup_time_us < 0) {
		DRM_DEBUG_KMS("PSR condition failed: Invalid PSR setup time (0x%02x)\n",
			    phytium_dp->psr_dpcd[1]);
	}

}

static void phytium_psr_enable_sink(struct phytium_dp_device *phytium_dp)
{
	uint8_t dpcd_val = DP_PSR_ENABLE;
	int ret = 0;

#if PSR_DEBUG
	pr_info("into phytium_psr_enable_sink\n");
#endif

	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_PSR_EN_CFG, dpcd_val);
	if (ret < 0) {
		DRM_ERROR("write DP_PSR_EN_CFG fail: ret:%d\n", ret);
		goto failed;
	}

	drm_dp_dpcd_writeb(&phytium_dp->aux, DP_SET_POWER, DP_SET_POWER_D0);

failed:
	return;
}

static void phytium_psr_enable_source(struct phytium_dp_device *phytium_dp)
{
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t val = 0;

#if PSR_DEBUG
	pr_info("into phytium_psr_enable_source\n");
#endif

	phytium_writel_reg(priv, val, group_offset, PHYTIUM_DP_SEC_PSR_CONFIG);
}

static void phytium_psr_activate(struct phytium_dp_device *phytium_dp)
{
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	int ret;
	ssize_t sink_status;
	ssize_t source_status;

#if PSR_DEBUG
	pr_info("into phytium_psr_activate\n");
#endif

	/* RESET SECONDARY STREAM state*/
	phytium_writel_reg(priv, SECONDARY_STREAM_DISABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	udelay(10);
	phytium_writel_reg(priv, SECONDARY_STREAM_ENABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);

	phytium_writel_reg(priv, PSR_ENABLE,
			   group_offset, PHYTIUM_DP_SEC_PSR_3D_ENABLE);
	phytium_writel_reg(priv, PSR_ACTIVE,
			   group_offset, PHYTIUM_DP_SEC_PSR_STATE);
	ret = readx_poll_timeout(phytium_dp_get_source_psr_status, phytium_dp, source_status,
		source_status >= 0 &&
		(source_status == PSR_STATE_ACTIVE),
		10000, 50000);

	if (ret) {
#if PSR_DEBUG
		pr_err("Timeout for source to transition from inactive to active\n");
		pr_err("Source state is %ld\n", source_status);
#endif
	}

	ret = readx_poll_timeout(phytium_dp_get_sink_psr_status, phytium_dp, sink_status,
		sink_status >= 0 &&
		(sink_status == DP_PSR_SINK_ACTIVE_RFB),
		10000, 50000);

	if (ret) {
#if PSR_DEBUG
		pr_err("Timeout for sink to transition from inactive to display from RFB\n");
		pr_err("Sink state is %ld\n", sink_status);
#endif
	}

	phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,
					group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);

	source_status = phytium_readl_reg(priv, group_offset, SEC_PSR_STATE_INTERNAL);
	if (!(source_status & PSR_STATE_ACTIVE))
		DRM_ERROR("Source PSR isn't active, state is %ld\n", source_status);

}

void phytium_psr_enable(struct phytium_dp_device *phytium_dp)
{
#if PSR_DEBUG
	uint8_t val;

	pr_info("into phytium_psr_enable\n");
#endif
	if (phytium_dp->psr.enabled) {
		DRM_DEBUG_KMS("PSR already in use\n");
		return;
	}

	phytium_psr_enable_sink(phytium_dp);
	phytium_psr_enable_source(phytium_dp);

	phytium_psr_activate(phytium_dp);
#if PSR_DEBUG
	drm_dp_dpcd_readb(&phytium_dp->aux, DP_LAST_RECEIVED_PSR_SDP, &val);
	pr_err("LAST RECEIVED PSR SDP: 0X%02x in %s\n", val, __func__);
#endif

	/* config lane active power state */
	phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, false);

	phytium_dp->psr.enabled = true;
}

static void phytium_psr_disable_source(struct phytium_dp_device *phytium_dp)
{
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	int ret;
	ssize_t source_status;

#if PSR_DEBUG
	pr_info("into phytium_psr_disable_source\n");
#endif

	phytium_writel_reg(priv, PSR_INACTIVE, group_offset, PHYTIUM_DP_SEC_PSR_STATE);
	ret = readx_poll_timeout(phytium_dp_get_source_psr_status, phytium_dp, source_status,
	source_status >= 0 &&
	(source_status == PSR_STATE_INACTIVE),
	10000, 100000);

	if (ret) {
#if PSR_DEBUG
		pr_err("Timeout for Source to transition from active to inactive\n");
		pr_err("Source state is %ld\n", source_status);
#endif
	}

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	if (phytium_dp->psr.sink_need_train_on_exit) {
		if (phytium_dp->fast_train_support)
			phytium_dp_fast_link_train(phytium_dp);
		else
			phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
		phytium_dp_fast_link_train_detect(phytium_dp);
	}
#endif
	phytium_writel_reg(priv, SST_MST_SOURCE_0_ENABLE,
					group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
}

void phytium_psr_sf_update(struct phytium_dp_device *phytium_dp)
{
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0;
	uint32_t group_offset = priv->dp_reg_base[port];
#if PSR_DEBUG
	uint32_t config;
#endif
	ssize_t psr_status;

#if PSR_DEBUG
	pr_info("into %s\n", __func__);
#endif

	drm_dp_dpcd_writeb(&phytium_dp->aux, DP_SET_POWER, DP_SET_POWER_D0);
	/* enable PHY and DP transmit */
	phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, true);

	if (phytium_dp->psr.sink_need_train_on_exit) {
		if (phytium_dp->fast_train_support)
			phytium_dp_fast_link_train(phytium_dp);
		else
			phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
		phytium_dp_fast_link_train_detect(phytium_dp);
	}

	phytium_writel_reg(priv, SST_MST_SOURCE_0_ENABLE,
					group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);

	/*Enable single frame update*/
	phytium_writel_reg(priv,  PSR_SINGLE_FRAME_UPDATE, group_offset, PHYTIUM_DP_SEC_PSR_STATE);

	/*wait for source state transform down*/
	ret = readx_poll_timeout(phytium_dp_get_source_psr_status, phytium_dp, psr_status,
		psr_status >= 0 &&
		(psr_status == PSR_STATE_ACTIVE_UPDATE),
		20000, 100000);
	if (ret)
		DRM_ERROR("Failed to apply PSR single frame update to source ret=%d\n", ret);
#if PSR_DEBUG
	pr_info("Source PSR state is %ld on %d\n", psr_status, __LINE__);
#endif

	/*wait for RFB capture done*/
	mdelay(100);
	ret = readx_poll_timeout(phytium_dp_get_sink_psr_status, phytium_dp, psr_status,
		psr_status >= 0 &&
		(psr_status && DP_PSR_SINK_STATE_MASK == DP_PSR_SINK_ACTIVE_SINK_SYNCED),
		20000, 1000000);
	if (ret)
		DRM_ERROR("Failed to apply PSR single frame update %d\n", ret);
#if PSR_DEBUG
	pr_info("sink state = %ld in %s\n", psr_status, __func__);
#endif

	/*Disable single frame update*/
	phytium_writel_reg(priv, PSR_ACTIVE, group_offset, PHYTIUM_DP_SEC_PSR_STATE);
	mdelay(100);

#if PSR_DEBUG
	config = phytium_readl_reg(priv, group_offset, SEC_PSR_STATE_INTERNAL);
	pr_info("Source PSR state is %d on %d\n", config, __LINE__);
#endif
	/* disable PHY and DP transmit */
	phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,
					group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);

	phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, false);
#endif

}

void phytium_psr_disable(struct phytium_dp_device *phytium_dp)
{
	int ret = 0;
	ssize_t sink_status;
#if PSR_DEBUG
	uint8_t val;

	pr_info("into phytium_psr_disable\n");
#endif

	if (!phytium_dp->psr.enabled) {
		DRM_DEBUG_KMS("PSR already disabled\n");
		return;
	}

	/* config lane active power state */
	phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, true);
	drm_dp_dpcd_writeb(&phytium_dp->aux, DP_SET_POWER, DP_SET_POWER_D0);

	phytium_psr_disable_source(phytium_dp);
	ret = readx_poll_timeout(phytium_dp_get_sink_psr_status, phytium_dp, sink_status,
		sink_status >= 0 &&
		(sink_status == DP_PSR_SINK_INACTIVE),
		20000, 100000);
#if PSR_DEBUG
	if (ret) {
		pr_err("Timeout for sink to transition from display from RFB to inactive\n");
		pr_err("Sink state is %ld\n", sink_status);
	}

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_LAST_RECEIVED_PSR_SDP, &val);
	pr_err("LAST RECEIVED PSR SDP: 0X%02x in %s\n", val, __func__);
#endif

	/* Disable PSR on Sink */
	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_PSR_EN_CFG, 0);
	if (ret < 0)
		DRM_ERROR("write DP_PSR_EN_CFG fail: ret:%d\n", ret);

	phytium_dp->psr.enabled = false;
}

void phytium_psr_exit_sf_detect(struct phytium_dp_device *phytium_dp)
{
	unsigned long start = phytium_dp->psr.last_jiffies;
	unsigned long end = start + msecs_to_jiffies(100);
	int threshold = 5;

	if (!time_in_range(jiffies, start, end)) {
		phytium_dp->psr.last_jiffies = jiffies;
	} else if (phytium_dp->psr.frame_change_irq_nums > threshold &&
			threshold) {
		phytium_dp->dplp_frame_compare_state.psr_exit = true;
	}
}

static int psr_get_status_and_error_status(struct phytium_dp_device *phytium_dp,
					   u8 *status, u8 *error_status)
{
	int ret;

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_STATUS, status);
	if (ret != 1)
		return ret;

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_ERROR_STATUS, error_status);
	if (ret != 1)
		return ret;

	*status = *status & DP_PSR_SINK_STATE_MASK;

	return 0;
}

static void psr_capability_changed_check(struct phytium_dp_device *phytium_dp)
{
	u8 val;
	int ret;

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_PSR_ESI, &val);
	if (ret != 1) {
		DRM_ERROR("Error reading DP_PSR_ESI\n");
		return;
	}

	if (val & DP_PSR_CAPS_CHANGE) {
		phytium_psr_disable(phytium_dp);
		phytium_dp->psr.sink_not_reliable = true;
		pr_err("Sink PSR capability changed, disabling PSR\n");

		/* Clearing it */
		drm_dp_dpcd_writeb(&phytium_dp->aux, DP_PSR_ESI, val);
	}
}

bool phytium_psr_short_pulse(struct phytium_dp_device *phytium_dp)
{
	u8 status, error_status;
	const u8 errors = DP_PSR_RFB_STORAGE_ERROR |
			  DP_PSR_VSC_SDP_UNCORRECTABLE_ERROR |
			  DP_PSR_LINK_CRC_ERROR;

	pr_info("FTD330 %s: %d\n", __func__, __LINE__);
	if (!phytium_dp->psr.sink_support)
		return 0;

	if (!phytium_dp->psr.enabled)
		return 0;

	if (psr_get_status_and_error_status(phytium_dp, &status, &error_status)) {
		DRM_ERROR("Error reading PSR status or error status\n");
		return 0;
	}

	if (status == DP_PSR_SINK_INTERNAL_ERROR || (error_status & errors)) {
		phytium_psr_disable(phytium_dp);
		phytium_dp->psr.sink_not_reliable = true;
	} else 
		return 0;

	if (status == DP_PSR_SINK_INTERNAL_ERROR && !error_status)
		pr_err("PSR sink internal error, disabling PSR\n");
	if (error_status & DP_PSR_RFB_STORAGE_ERROR)
		pr_err("PSR RFB storage error, disabling PSR\n");
	if (error_status & DP_PSR_VSC_SDP_UNCORRECTABLE_ERROR)
		pr_err("PSR VSC SDP uncorrectable error, disabling PSR\n");
	if (error_status & DP_PSR_LINK_CRC_ERROR)
		pr_err("PSR Link CRC error, disabling PSR\n");

	if (error_status & ~errors)
		DRM_ERROR("PSR_ERROR_STATUS unhandled errors %x\n",
			error_status & ~errors);
	/* clear status register */
	drm_dp_dpcd_writeb(&phytium_dp->aux, DP_PSR_ERROR_STATUS, error_status);

	psr_capability_changed_check(phytium_dp);

	return 1;
}