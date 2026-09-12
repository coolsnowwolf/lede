// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */
#include <linux/version.h>
#include <drm/drm_atomic_helper.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
#include <drm/display/drm_dp.h>
#else
#include <drm/drm_dp_helper.h>
#endif
#include "ftd330_dc.h"
#include "ftd330_crtc.h"
#include "phytium_dp.h"
#include "phytium_dp_reg.h"
#include "phytium_vrr.h"

#define VRR_DEBUG 0

bool
phytium_sink_supports_vrr(struct phytium_dp_device *phytium_dp)
{
	struct edid *edid;
	struct detailed_timing *timing;
	struct detailed_non_pixel *data;
	struct drm_connector *connector = &phytium_dp->connector;

	uint8_t msa_ignore = 0;
	bool valid_range = false;
	bool support = false;
	int i;

	if (!connector) {
		DRM_INFO("Connector is null\n");
		return support;
	}
	/*
	 * First read dpcd 0x007 bit 6 to determine sink supports ignoring MSA
	 */
	drm_dp_dpcd_readb(&phytium_dp->aux, DP_DOWN_STREAM_PORT_COUNT, &msa_ignore);
	if (!(msa_ignore & DP_MSA_TIMING_PAR_IGNORED)) {
		DRM_INFO("Sink can not ignore MSA!\n");
		return support;
	}

	/*
	 * Then read edid to determine sink support continuous freq
	 */
	if (!drm_edid_is_valid(phytium_dp->edp_edid))
		edid = drm_get_edid(connector, &phytium_dp->aux.ddc);
	else
		edid = phytium_dp->edp_edid;
	if (drm_edid_is_valid(edid)) {
		if (!(edid->features & DRM_EDID_FEATURE_DEFAULT_GTF)) {
			DRM_INFO("Display is non-continuous frequency\n");
			return support;
		}

		for (i = 0; i < 4; i++) {
			timing	= &edid->detailed_timings[i];
			data	= &timing->data.other_data;
			phytium_dp->vrr.range = &data->data.range;
			/*
			 * Check if monitor has continuous frequency mode
			 */
			if (data->type != EDID_DETAIL_MONITOR_RANGE)
				continue;
			valid_range = true;
			support = true;
			break;
		}
		if (!valid_range)
			DRM_INFO("EDID does not provide a display range limit");
	}
	return support;
}

static void
phytium_calcu_vrr_mode_config(struct dc_hw_display_mode *vrr_mode,
							  struct dc_hw_display_mode old_mode,
							  const void *data)
{
	const u32 vrr_refresh = *(u32 *)data;
	u32 new_vtotal = 0, new_vfp = 0, vsync_width = 0, vbp = 0;

	/* calculate vrr display mode */
	vsync_width = old_mode.v_sync_end - old_mode.v_sync_start;
	vbp = old_mode.v_total - old_mode.v_sync_end;
	/* clock = v_total * h_total * refresh , not the real hardware situation */
	new_vtotal = _DIV_ROUND_CLOSEST(old_mode.clock * 1000, old_mode.h_total * vrr_refresh);
	new_vfp = new_vtotal - old_mode.v_active - vsync_width - vbp;

	vrr_mode->v_active = old_mode.v_active;
	vrr_mode->v_sync_start = vrr_mode->v_active + new_vfp;
	vrr_mode->v_sync_end = vrr_mode->v_sync_start + vsync_width;
	vrr_mode->v_total = new_vtotal;
}

static void
phytium_vrr_mode_config_hw_dc(struct dc_hw *hw, uint8_t hw_id,
								struct dc_hw_display_mode *vrr_mode)
{
	u32 display_offset = display_get_addr_offset(hw, hw_id);

	dc_set_clear(hw, DC_DISPLAY_V + display_offset,
			vrr_mode->v_active | (vrr_mode->v_total << 16),
			GENMASK(14, 0) | GENMASK(30, 16));
	dc_set_clear(hw, DC_DISPLAY_V_SYNC + display_offset,
			vrr_mode->v_sync_start | (vrr_mode->v_sync_end << 15), GENMASK(29, 0));
#if DEBUG_LOG
	pr_info("write vrr_mode->vtotal: h0x%08x to DC_DISPLAY_V: h0x%x\n",
			vrr_mode->v_total, DC_DISPLAY_V + display_offset);
#endif
}

static void
phytium_vrr_mode_config_hw_dp(struct phytium_dp_device *phytium_dp,
								struct dc_hw_display_mode *vrr_mode)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	/* config main stream attributes */
	phytium_writel_reg(priv, vrr_mode->v_total,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VTOTAL);
	phytium_writel_reg(priv, vrr_mode->v_active,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VRES);
	phytium_writel_reg(priv,
			   vrr_mode->v_sync_end - vrr_mode->v_sync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VSWIDTH);
	phytium_writel_reg(priv, vrr_mode->v_total - vrr_mode->v_sync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VSTART);
#if DEBUG_LOG
	pr_info("write vrr_mode->vtotal: h0x%08x to PHYTIUM_DP_MAIN_LINK_VTOTAL\n",
				vrr_mode->v_total);
#endif
}

void phytium_change_fps(struct phytium_dp_device *phytium_dp, bool enable, int freq)
{
	struct drm_device *dev = NULL;
	struct ftd330_drm_private *priv = NULL;
	struct ftd330_dc *dc = NULL;
	struct drm_connector *connector = NULL;
	struct drm_crtc *crtc = NULL;
	struct ftd330_crtc *ftd330_crtc = NULL;
	struct dc_hw *hw = NULL;
	uint8_t min_vfreq = 0, max_vfreq = 0;
	struct dc_hw_display_mode old_mode;
	struct dc_hw_display_mode vrr_mode;
	unsigned int num, den;

	dev = phytium_dp->dev;
	if (!dev) {
		DRM_INFO("%s: dev is null\n", __func__);
		return;
	}

	priv = dev->dev_private;
	if (!priv) {
		DRM_INFO("%s: priv is null\n", __func__);
		return;
	}

	dc = dev_get_drvdata(priv->dc_dev);
	if (!dc) {
		DRM_INFO("%s: dc is null\n", __func__);
		return;
	}

	connector = &phytium_dp->connector;
	if (!connector) {
		DRM_INFO("%s: connector is null\n", __func__);
		return;
	}

	crtc = connector->state->crtc;
	if (!crtc) {
		DRM_INFO("%s: crtc is null\n", __func__);
		return;
	}

	ftd330_crtc = to_ftd330_crtc(crtc);
	if (!ftd330_crtc) {
		DRM_INFO("%s: ftd330_crtc is null\n", __func__);
		return;
	}

	hw = &dc->hw;
	if (!hw) {
		DRM_INFO("%s: hw is null\n", __func__);
		return;
	}

	old_mode = hw->display[ftd330_crtc->id].mode;
	num = old_mode.clock;
	den = old_mode.h_total * old_mode.v_total;

	if (!enable) {
#if VRR_DEBUG
		pr_info("disable vrr\n");
#endif
		phytium_vrr_mode_config_hw_dc(hw, ftd330_crtc->id, &old_mode);
		phytium_vrr_mode_config_hw_dp(phytium_dp, &old_mode);
		phytium_dp->vrr.enabled = false;
		phytium_dp->freq = DIV_ROUND_CLOSEST_ULL(mul_u32_u32(num, 1000), den);
		return;
	}

	if (!phytium_dp->vrr.sink_support) {
		DRM_INFO("Sink doesn't support vrr\n");
		return;
	}

	min_vfreq = phytium_dp->vrr.range->min_vfreq;
	max_vfreq = phytium_dp->vrr.range->max_vfreq;
	if (!freq)
		freq = min_vfreq;
	else if (freq < min_vfreq || freq > max_vfreq) {
		DRM_INFO("vfreq %d Hz is not in the supported range %d-%d\n",
				  freq, min_vfreq, max_vfreq);
		return;
	}

	phytium_calcu_vrr_mode_config(&vrr_mode, old_mode, &freq);

	phytium_vrr_mode_config_hw_dc(hw, ftd330_crtc->id, &vrr_mode);
	phytium_vrr_mode_config_hw_dp(phytium_dp, &vrr_mode);
	phytium_dp->freq = freq;
	phytium_dp->vrr.enabled = true;
#if VRR_DEBUG
		pr_info("enable vrr\n");
#endif
}
