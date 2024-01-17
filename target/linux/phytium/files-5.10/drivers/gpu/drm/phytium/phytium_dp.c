// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/version.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_dp_helper.h>
#include <drm/drm_encoder.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_modes.h>
#include <sound/hdmi-codec.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <drm/drm_probe_helper.h>
#endif
#include "phytium_display_drv.h"
#include "phytium_dp.h"
#include "phytium_debugfs.h"
#include "px210_dp.h"
#include "pe220x_dp.h"
#include "phytium_panel.h"
#include "phytium_reg.h"

static void phytium_dp_aux_init(struct phytium_dp_device *phytium_dp);
static void handle_plugged_change(struct phytium_dp_device *phytium_dp, bool plugged);
static bool phytium_edp_init_connector(struct phytium_dp_device *phytium_dp);
static void phytium_edp_fini_connector(struct phytium_dp_device *phytium_dp);
static void phytium_edp_panel_poweroff(struct phytium_dp_device *phytium_dp);
static void phytium_dp_audio_codec_fini(struct phytium_dp_device *phytium_dp);

static int phytium_rate[] = {162000, 270000, 540000, 810000};
static int codec_id = PHYTIUM_DP_AUDIO_ID;

void phytium_phy_writel(struct phytium_dp_device *phytium_dp, uint32_t address, uint32_t data)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->phy_access_base[port];

#if DEBUG_LOG
	pr_info("phy address write: 0x%x data:0x%x\n", address, data);
#endif
	phytium_writel_reg(priv, address, group_offset, PHYTIUM_PHY_ACCESS_ADDRESS);
	phytium_writel_reg(priv, data, group_offset, PHYTIUM_PHY_WRITE_DATA);
	phytium_writel_reg(priv, ACCESS_WRITE, group_offset, PHYTIUM_PHY_ACCESS_CTRL);
	udelay(10);
}

uint32_t phytium_phy_readl(struct phytium_dp_device *phytium_dp, uint32_t address)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->phy_access_base[port];
	uint32_t data;

	phytium_writel_reg(priv, address, group_offset, PHYTIUM_PHY_ACCESS_ADDRESS);
	phytium_writel_reg(priv, ACCESS_READ, group_offset, PHYTIUM_PHY_ACCESS_CTRL);
	udelay(10);
	data = phytium_readl_reg(priv, group_offset, PHYTIUM_PHY_READ_DATA);
#if DEBUG_LOG
	pr_info("phy address read: 0x%x data:0x%x\n", address, data);
#endif

	return data;
}

static int
phytium_dp_hw_aux_transfer_write(struct phytium_dp_device *phytium_dp, struct drm_dp_aux_msg *msg)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	unsigned int i = 0, j = 0;
	unsigned int cmd = 0;
	unsigned int aux_status = 0, interrupt_status = 0;
	unsigned char *data = msg->buffer;
	int count_timeout = 0;
	long ret = 0;

	for (i = 0; i < 3; i++) {
		/* clear PX210_DP_INTERRUPT_RAW_STATUS */
		phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
		phytium_writel_reg(priv, msg->address, group_offset, PHYTIUM_DP_AUX_ADDRESS);
		for (j = 0; j < msg->size; j++)
			phytium_writeb_reg(priv, data[j], group_offset, PHYTIUM_DP_AUX_WRITE_FIFO);

		cmd = ((msg->request & COMMAND_MASK) << COMMAND_SHIFT);
		if (msg->size == 0)
			cmd |= ADDRESS_ONLY;
		else
			cmd |= (msg->size-1) & BYTE_COUNT_MASK;
		phytium_writel_reg(priv, cmd, group_offset, PHYTIUM_DP_AUX_COMMAND);

		count_timeout = 0;
		do {
			mdelay(5);
			interrupt_status = phytium_readl_reg(priv, group_offset,
							PHYTIUM_DP_INTERRUPT_RAW_STATUS);
			aux_status = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_STATUS);
			if ((aux_status & REPLY_RECEIVED) || (aux_status & REPLY_ERROR)
			    || (interrupt_status & REPLY_TIMEOUT)) {
				DRM_DEBUG_KMS("aux wait exit\n");
				break;
			}
			count_timeout++;
		} while (count_timeout < 6);

		phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
		if (interrupt_status & REPLY_TIMEOUT) {
			DRM_DEBUG_KMS("aux write reply timeout\n");
			continue;
		} else if (aux_status & REPLY_ERROR) {
			DRM_DEBUG_KMS("aux write reply error\n");
			continue;
		} else if (aux_status & REPLY_RECEIVED) {
			DRM_DEBUG_KMS("aux write reply received succussful\n");
			break;
		}
	}

	if (interrupt_status & REPLY_TIMEOUT) {
		DRM_NOTE("aux(%d) write reply timeout\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	} else if (aux_status & REPLY_ERROR) {
		DRM_ERROR("aux(%d) write reply error\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	} else if ((aux_status & REPLY_RECEIVED) != REPLY_RECEIVED) {
		DRM_ERROR("aux(%d) write reply no response\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	}

	msg->reply = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_REPLY_CODE);
	ret = msg->size;
out:
	return ret;
}

static int
phytium_dp_hw_aux_transfer_read(struct phytium_dp_device *phytium_dp, struct drm_dp_aux_msg *msg)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	unsigned int i = 0;
	unsigned int cmd = 0;
	unsigned int aux_status = 0, interrupt_status = 0;
	unsigned char *data = msg->buffer;
	int count_timeout = 0;
	long ret = 0;

	for (i = 0; i < 3; i++) {
		phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
		phytium_writel_reg(priv, msg->address, group_offset, PHYTIUM_DP_AUX_ADDRESS);
		cmd = ((msg->request & COMMAND_MASK) << COMMAND_SHIFT);
		if (msg->size == 0)
			cmd |= ADDRESS_ONLY;
		else
			cmd |= ((msg->size-1) & BYTE_COUNT_MASK);
		phytium_writel_reg(priv, cmd, group_offset, PHYTIUM_DP_AUX_COMMAND);

		count_timeout = 0;
		do {
			mdelay(5);
			interrupt_status = phytium_readl_reg(priv, group_offset,
							     PHYTIUM_DP_INTERRUPT_RAW_STATUS);
			aux_status = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_STATUS);
			if ((aux_status & REPLY_RECEIVED) || (aux_status & REPLY_ERROR)
			   || (interrupt_status & REPLY_TIMEOUT)) {
				DRM_DEBUG_KMS("aux wait exit\n");
				break;
			}
			count_timeout++;
		} while (count_timeout < 6);

		phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
		if (interrupt_status & REPLY_TIMEOUT) {
			DRM_DEBUG_KMS("aux read reply timeout\n");
			continue;
		} else if (aux_status & REPLY_ERROR) {
			DRM_DEBUG_KMS("aux read reply error\n");
			continue;
		} else if (aux_status & REPLY_RECEIVED) {
			DRM_DEBUG_KMS("aux read reply received succussful\n");
			break;
		}
	}

	if (interrupt_status & REPLY_TIMEOUT) {
		DRM_NOTE("aux(%d) read reply timeout\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	} else if (aux_status & REPLY_ERROR) {
		DRM_ERROR("aux(%d) read reply error\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	} else if ((aux_status & REPLY_RECEIVED) != REPLY_RECEIVED) {
		DRM_ERROR("aux(%d) read reply no response\n", phytium_dp->port);
		ret = -EIO;
		goto out;
	}

	msg->reply = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_REPLY_CODE);
	ret = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);

	if (ret > msg->size) {
		ret = msg->size;
	} else if (ret != msg->size) {
		DRM_DEBUG_KMS("aux read count error(ret:0x%lx != 0x%lx)\n", ret, msg->size);
		ret = -EBUSY;
		goto out;
	}

	for (i = 0; i < ret; i++)
		data[i] = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_REPLY_DATA);

out:
	return ret;
}

static void phytium_get_native_mode(struct phytium_dp_device *phytium_dp)
{
	struct drm_display_mode *t, *mode;
	struct drm_connector *connector = &phytium_dp->connector;
	struct drm_display_mode *native_mode = &phytium_dp->native_mode;

	list_for_each_entry_safe(mode, t, &connector->probed_modes, head) {
		if (mode->type & DRM_MODE_TYPE_PREFERRED) {
			if (mode->hdisplay != native_mode->hdisplay ||
			    mode->vdisplay != native_mode->vdisplay) {
				memcpy(native_mode, mode, sizeof(*mode));
				drm_mode_set_crtcinfo(native_mode, 0);
			}
			break;
		}
	}

	if (&mode->head == &connector->probed_modes)
		native_mode->clock = 0;
}

static int phytium_connector_add_common_modes(struct phytium_dp_device *phytium_dp)
{
	int i = 0, ret = 0;
	struct drm_device *dev =  phytium_dp->dev;
	struct drm_display_mode *mode = NULL, *current_mode = NULL;
	struct drm_display_mode *native_mode = &phytium_dp->native_mode;
	bool mode_existed = false;
	struct mode_size {
		char name[DRM_DISPLAY_MODE_LEN];
		int w;
		int h;
	} common_mode[] = {
		{  "640x480",  640,  480},
		{  "800x600",  800,  600},
		{ "1024x768", 1024,  768},
		{ "1280x720", 1280,  720},
		{ "1280x800", 1280,  800},
		{"1280x1024", 1280, 1024},
		{ "1440x900", 1440,  900},
		{"1680x1050", 1680, 1050},
		{"1600x1200", 1600, 1200},
		{"1920x1080", 1920, 1080},
		{"1920x1200", 1920, 1200}
	};

	if (native_mode->clock == 0)
		return ret;

	for (i = 0; i < ARRAY_SIZE(common_mode); i++) {
		mode_existed = false;

		if (common_mode[i].w > native_mode->hdisplay ||
		    common_mode[i].h > native_mode->vdisplay ||
		    (common_mode[i].w == native_mode->hdisplay &&
		    common_mode[i].h == native_mode->vdisplay))
			continue;

		list_for_each_entry(current_mode, &phytium_dp->connector.probed_modes, head) {
			if (common_mode[i].w == current_mode->hdisplay &&
			    common_mode[i].h == current_mode->vdisplay) {
				mode_existed = true;
				break;
			}
		}

		if (mode_existed)
			continue;

		mode = drm_mode_duplicate(dev, native_mode);
		if (mode == NULL)
			continue;

		mode->hdisplay = common_mode[i].w;
		mode->vdisplay = common_mode[i].h;
		mode->type &= ~DRM_MODE_TYPE_PREFERRED;
		strncpy(mode->name, common_mode[i].name, DRM_DISPLAY_MODE_LEN);
		drm_mode_probed_add(&phytium_dp->connector, mode);
		ret++;
	}

	return ret;
}

static int phytium_connector_get_modes(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct edid *edid;
	int ret = 0;

	if (phytium_dp->is_edp)
		edid = phytium_dp->edp_edid;
	else
		edid = drm_get_edid(connector, &phytium_dp->aux.ddc);

	if (edid && drm_edid_is_valid(edid)) {
		drm_connector_update_edid_property(connector, edid);
		ret = drm_add_edid_modes(connector, edid);
		phytium_dp->has_audio = drm_detect_monitor_audio(edid);
		phytium_get_native_mode(phytium_dp);
		if (dc_fake_mode_enable)
			ret += phytium_connector_add_common_modes(phytium_dp);
	} else {
		drm_connector_update_edid_property(connector, NULL);
		phytium_dp->has_audio = false;
	}

	if (!phytium_dp->is_edp)
		kfree(edid);

	return ret;
}

static struct drm_encoder *phytium_dp_best_encoder(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	return &phytium_dp->encoder;
}

static const
struct drm_connector_helper_funcs phytium_connector_helper_funcs = {
	.get_modes  = phytium_connector_get_modes,
	.best_encoder = phytium_dp_best_encoder,
};

static void phytium_dp_set_sink_rates(struct phytium_dp_device *phytium_dp)
{
	static const int dp_rates[] = {162000, 270000, 540000, 810000};
	int i, max_rate;

	max_rate = drm_dp_bw_code_to_link_rate(phytium_dp->dpcd[DP_MAX_LINK_RATE]);
	for (i = 0; i < ARRAY_SIZE(dp_rates); i++) {
		if (dp_rates[i] > max_rate)
			break;
		phytium_dp->sink_rates[i] = dp_rates[i];
	}
	phytium_dp->num_sink_rates = i;
}

static int get_common_rates(const int *source_rates, int source_len, const int *sink_rates,
				  int sink_len, int *common_rates)
{
	int i = 0, j = 0, k = 0;

	while (i < source_len && j < sink_len) {
		if (source_rates[i] == sink_rates[j]) {
			if (WARN_ON(k >= DP_MAX_SUPPORTED_RATES))
				return k;
			common_rates[k] = source_rates[i];
			++k;
			++i;
			++j;
		} else if (source_rates[i] < sink_rates[j]) {
			++i;
		} else {
			++j;
		}
	}
	return k;
}

static void phytium_dp_set_common_rates(struct phytium_dp_device *phytium_dp)
{
	WARN_ON(!phytium_dp->num_source_rates || !phytium_dp->num_sink_rates);

	phytium_dp->num_common_rates = get_common_rates(phytium_dp->source_rates,
							phytium_dp->num_source_rates,
							phytium_dp->sink_rates,
							phytium_dp->num_sink_rates,
							phytium_dp->common_rates);

	if (WARN_ON(phytium_dp->num_common_rates == 0)) {
		phytium_dp->common_rates[0] = 162000;
		phytium_dp->num_common_rates = 1;
	}
}

static bool phytium_dp_get_dpcd(struct phytium_dp_device *phytium_dp)
{
	int ret;
	unsigned char sink_count = 0;

	/* get dpcd capability,but don't check data error; so check revision */
	ret = drm_dp_dpcd_read(&phytium_dp->aux, 0x00, phytium_dp->dpcd,
			       sizeof(phytium_dp->dpcd));
	if (ret < 0) {
		DRM_ERROR("port %d get DPCD capability fail\n", phytium_dp->port);
		return false;
	}

	if (phytium_dp->dpcd[DP_DPCD_REV] == 0) {
		DRM_ERROR("DPCD data error: 0x%x\n", phytium_dp->dpcd[DP_DPCD_REV]);
		return false;
	}

	/* parse sink support link */
	phytium_dp_set_sink_rates(phytium_dp);
	phytium_dp_set_common_rates(phytium_dp);
	phytium_dp->sink_max_lane_count = drm_dp_max_lane_count(phytium_dp->dpcd);
	phytium_dp->common_max_lane_count = min(phytium_dp->source_max_lane_count,
						phytium_dp->sink_max_lane_count);

	/* get dpcd sink count */
	if (drm_dp_dpcd_readb(&phytium_dp->aux, DP_SINK_COUNT, &sink_count) <= 0) {
		DRM_ERROR("get DPCD sink_count fail\n");
		return false;
	}

	phytium_dp->sink_count = DP_GET_SINK_COUNT(sink_count);
	if (!phytium_dp->sink_count) {
		DRM_ERROR("DPCD sink_count should not be zero\n");
		return false;
	}

	if (!drm_dp_is_branch(phytium_dp->dpcd))
		return true;

	if (phytium_dp->dpcd[DP_DPCD_REV] == 0x10)
		return true;

	/* get downstream port for branch device */
	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_DOWNSTREAM_PORT_0,
			       phytium_dp->downstream_ports, DP_MAX_DOWNSTREAM_PORTS);
	if (ret < 0) {
		DRM_ERROR("get DPCD DFP fail\n");
		return false;
	}

	return true;
}

static enum drm_connector_status
phytium_dp_detect_dpcd(struct phytium_dp_device *phytium_dp)
{
	if (!phytium_dp_get_dpcd(phytium_dp))
		return connector_status_disconnected;

	if (!drm_dp_is_branch(phytium_dp->dpcd))
		return connector_status_connected;

	if (phytium_dp->downstream_ports[0] & DP_DS_PORT_HPD) {
		return phytium_dp->sink_count ? connector_status_connected
			: connector_status_disconnected;
	}
	return connector_status_connected;
}

static void phytium_get_adjust_train(struct phytium_dp_device *phytium_dp,
				const uint8_t link_status[DP_LINK_STATUS_SIZE], uint8_t lane_count)
{
	unsigned char v = 0;
	unsigned char p = 0;
	int lane;
	unsigned char voltage_max;
	unsigned char preemph_max;

	/* find max value */
	for (lane = 0; lane < lane_count; lane++) {
		uint8_t this_v = drm_dp_get_adjust_request_voltage(link_status, lane);
		uint8_t this_p = drm_dp_get_adjust_request_pre_emphasis(link_status, lane);

		if (this_v > v)
			v = this_v;
		if (this_p > p)
			p = this_p;
	}
	voltage_max = DP_TRAIN_VOLTAGE_SWING_LEVEL_3;
	if (v >= voltage_max)
		v = voltage_max | DP_TRAIN_MAX_SWING_REACHED;

	preemph_max = DP_TRAIN_PRE_EMPH_LEVEL_3;
	if (p >= preemph_max)
		p = preemph_max | DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

	for (lane = 0; lane < 4; lane++)
		phytium_dp->train_set[lane] = v | p;
}

bool phytium_dp_coding_8b10b_need_enable(unsigned char test_pattern)
{
	switch (test_pattern) {
	case PHYTIUM_PHY_TP_D10_2:
	case PHYTIUM_PHY_TP_SYMBOL_ERROR:
	case PHYTIUM_PHY_TP_CP2520_1:
	case PHYTIUM_PHY_TP_CP2520_2:
	case PHYTIUM_PHY_TP_CP2520_3:
		return true;
	case PHYTIUM_PHY_TP_PRBS7:
	case PHYTIUM_PHY_TP_80BIT_CUSTOM:
		return false;
	default:
		return false;
	}
}

bool phytium_dp_scrambled_need_enable(unsigned char test_pattern)
{
	switch (test_pattern) {
	case PHYTIUM_PHY_TP_SYMBOL_ERROR:
	case PHYTIUM_PHY_TP_CP2520_1:
	case PHYTIUM_PHY_TP_CP2520_2:
	case PHYTIUM_PHY_TP_CP2520_3:
		return true;
	case PHYTIUM_PHY_TP_D10_2:
	case PHYTIUM_PHY_TP_PRBS7:
	case PHYTIUM_PHY_TP_80BIT_CUSTOM:
		return false;
	default:
		return false;
	}
}

static void phytium_dp_hw_set_lane_setting(struct phytium_dp_device *phytium_dp,
							 uint32_t link_rate,
							 uint8_t train_set)
{
	phytium_dp->funcs->dp_hw_set_phy_lane_setting(phytium_dp, link_rate, train_set);
}

static void phytium_dp_hw_set_link(struct phytium_dp_device *phytium_dp,
					     uint8_t lane_count,
					     uint32_t link_rate)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, ret = 0, retry = 3;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, lane_count,
			   group_offset, PHYTIUM_DP_LANE_COUNT_SET);
	phytium_writel_reg(priv,
			   drm_dp_link_rate_to_bw_code(link_rate),
			   group_offset, PHYTIUM_DP_LINK_BW_SET);

	if (drm_dp_enhanced_frame_cap(phytium_dp->dpcd))
		phytium_writel_reg(priv, ENHANCED_FRAME_ENABLE,
				   group_offset, PHYTIUM_DP_ENHANCED_FRAME_EN);
	else
		phytium_writel_reg(priv, ENHANCED_FRAME_DISABLE,
				   group_offset, PHYTIUM_DP_ENHANCED_FRAME_EN);

try_again:
	ret = phytium_dp->funcs->dp_hw_set_phy_lane_and_rate(phytium_dp, lane_count, link_rate);
	if ((ret < 0) && retry) {
		retry--;
		goto try_again;
	}
}

static void phytium_dp_hw_set_test_pattern(struct phytium_dp_device *phytium_dp,
							 uint8_t lane_count,
							 uint8_t test_pattern,
							 uint8_t *custom_pattern,
							 uint32_t custom_pattern_size)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, val = 0, tmp = 0, i;
	uint32_t group_offset = priv->dp_reg_base[port];

	if ((test_pattern == PHYTIUM_PHY_TP_80BIT_CUSTOM)
	    && custom_pattern && (custom_pattern_size > 0)) {
		val = *(int *)custom_pattern;
		phytium_writel_reg(priv, val, group_offset, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_0);
		val = *(int *)(custom_pattern + 4);
		phytium_writel_reg(priv, val, group_offset, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_1);
		val = *(short int *)(custom_pattern + 8);
		phytium_writel_reg(priv, val, group_offset, PHYTIUM_DP_CUSTOM_80BIT_PATTERN_2);
	}

	if (test_pattern == PHYTIUM_PHY_TP_D10_2 || test_pattern == PHYTIUM_PHY_TP_PRBS7
		|| test_pattern == PHYTIUM_PHY_TP_80BIT_CUSTOM)
		phytium_writel_reg(priv, SCRAMBLING_DISABLE, group_offset,
				   PHYTIUM_DP_SCRAMBLING_DISABLE);
	else
		phytium_writel_reg(priv, SCRAMBLING_ENABLE, group_offset,
				   PHYTIUM_DP_SCRAMBLING_DISABLE);

	tmp = test_pattern - PHYTIUM_PHY_TP_NONE + TEST_PATTERN_NONE;
	val = 0;
	for (i = 0; i < lane_count; i++)
		val |= (tmp << (TEST_PATTERN_LANE_SHIFT * i));
	phytium_writel_reg(priv, val, group_offset, PHYTIUM_DP_LINK_QUAL_PATTERN_SET);
}

static void phytium_dp_hw_set_train_pattern(struct phytium_dp_device *phytium_dp,
							  uint8_t train_pattern)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, tmp = 0;
	uint32_t group_offset = priv->dp_reg_base[port];

	/* Scrambling is disabled for TPS1/TPS2/3 and enabled for TPS4 */
	if (train_pattern == DP_TRAINING_PATTERN_4
	    || train_pattern == DP_TRAINING_PATTERN_DISABLE) {
		phytium_writel_reg(priv, SCRAMBLING_ENABLE, group_offset,
				   PHYTIUM_DP_SCRAMBLING_DISABLE);
		phytium_writel_reg(priv, SCRAMBLER_RESET, group_offset,
				   PHYTIUM_DP_FORCE_SCRAMBLER_RESET);
	} else {
		phytium_writel_reg(priv, SCRAMBLING_DISABLE, group_offset,
				   PHYTIUM_DP_SCRAMBLING_DISABLE);
	}
	switch (train_pattern) {
	case DP_TRAINING_PATTERN_DISABLE:
		tmp = TRAINING_OFF;
		break;
	case DP_TRAINING_PATTERN_1:
		tmp = TRAINING_PATTERN_1;
		break;
	case DP_TRAINING_PATTERN_2:
		tmp = TRAINING_PATTERN_2;
		break;
	case DP_TRAINING_PATTERN_3:
		tmp = TRAINING_PATTERN_3;
		break;
	case DP_TRAINING_PATTERN_4:
		tmp = TRAINING_PATTERN_4;
		break;
	default:
		tmp = TRAINING_OFF;
		break;
	}

	phytium_writel_reg(priv, tmp, group_offset, PHYTIUM_DP_TRAINING_PATTERN_SET);
}

void phytium_dp_hw_enable_audio(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int config = 0, config1, data_window = 0;
	const struct dp_audio_n_m *n_m = NULL;
	uint32_t group_offset = priv->dp_reg_base[port];

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);
	phytium_writel_reg(priv, CHANNEL_MUTE_ENABLE, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);

	data_window = 90*(phytium_dp->link_rate)/100
			*(phytium_dp->mode.htotal - phytium_dp->mode.hdisplay)
			/phytium_dp->mode.clock/4;

	phytium_writel_reg(priv, data_window, group_offset, PHYTIUM_DP_SEC_DATA_WINDOW);

	n_m = phytium_dp_audio_get_n_m(phytium_dp->link_rate, phytium_dp->audio_info.sample_rate);
	if (n_m == NULL) {
		DRM_NOTE("can not get n_m for link_rate(%d) and sample_rate(%d)\n",
				phytium_dp->link_rate, phytium_dp->audio_info.sample_rate);
		phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_SEC_MAUD);
		phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_SEC_NAUD);
	} else {
		phytium_writel_reg(priv, n_m->m, group_offset, PHYTIUM_DP_SEC_MAUD);
		phytium_writel_reg(priv, n_m->n, group_offset, PHYTIUM_DP_SEC_NAUD);
	}

	config1 = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	phytium_writel_reg(priv, SECONDARY_STREAM_DISABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	phytium_writel_reg(priv, config1, group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);
}

static void phytium_dp_hw_audio_shutdown(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SECONDARY_STREAM_DISABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
}

static void phytium_dp_hw_audio_digital_mute(struct phytium_dp_device *phytium_dp, bool enable)
{
	struct phytium_display_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	if (enable)
		phytium_writel_reg(priv, CHANNEL_MUTE_ENABLE,
				   group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);
	else
		phytium_writel_reg(priv, SEC_AUDIO_ENABLE,
				   group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);
}

static int
phytium_dp_hw_audio_hw_params(struct phytium_dp_device *phytium_dp, struct audio_info audio_info)
{
	struct phytium_display_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0, data_window = 0;
	const struct dp_audio_n_m *n_m = NULL;
	uint32_t fs, ws, fs_accurac;
	uint32_t group_offset = priv->dp_reg_base[port];

	DRM_DEBUG_KMS("%s:set port%d sample_rate(%d) channels(%d) sample_width(%d)\n",
			__func__, phytium_dp->port, audio_info.sample_rate,
			audio_info.channels, audio_info.sample_width);

	phytium_writel_reg(priv, INPUT_SELECT_I2S, group_offset, PHYTIUM_DP_SEC_INPUT_SELECT);
	phytium_writel_reg(priv, APB_CLOCK/audio_info.sample_rate,
			   group_offset, PHYTIUM_DP_SEC_DIRECT_CLKDIV);
	phytium_writel_reg(priv, audio_info.channels & CHANNEL_MASK,
			   group_offset, PHYTIUM_DP_SEC_CHANNEL_COUNT);
	phytium_writel_reg(priv, CHANNEL_MAP_DEFAULT, group_offset, PHYTIUM_DP_SEC_CHANNEL_MAP);
	data_window = 90*(phytium_dp->link_rate)/100
			*(phytium_dp->mode.htotal - phytium_dp->mode.hdisplay)
			/phytium_dp->mode.clock/4;
	phytium_writel_reg(priv, data_window, group_offset, PHYTIUM_DP_SEC_DATA_WINDOW);
	phytium_writel_reg(priv, 0xb5, group_offset, PHYTIUM_DP_SEC_CS_CATEGORY_CODE);

	phytium_writel_reg(priv, CLOCK_MODE_SYNC, group_offset, PHYTIUM_DP_SEC_CLOCK_MODE);
	phytium_writel_reg(priv, CS_SOURCE_FORMAT_DEFAULT,
			   group_offset, PHYTIUM_DP_SEC_CS_SOURCE_FORMAT);

	switch (audio_info.sample_rate) {
	case 32000:
		fs = ORIG_FREQ_32000;
		fs_accurac = SAMPLING_FREQ_32000;
		break;
	case 44100:
		fs = ORIG_FREQ_44100;
		fs_accurac = SAMPLING_FREQ_44100;
		break;
	case 48000:
		fs = ORIG_FREQ_48000;
		fs_accurac = SAMPLING_FREQ_48000;
		break;
	case 96000:
		fs = ORIG_FREQ_96000;
		fs_accurac = SAMPLING_FREQ_96000;
		break;
	case 176400:
		fs = ORIG_FREQ_176400;
		fs_accurac = SAMPLING_FREQ_176400;
		break;
	case 192000:
		fs = ORIG_FREQ_192000;
		fs_accurac = SAMPLING_FREQ_192000;
		break;
	default:
		DRM_ERROR("dp not support sample_rate %d\n", audio_info.sample_rate);
		goto out;
	}

	switch (audio_info.sample_width) {
	case 16:
		ws = WORD_LENGTH_16;
		break;
	case 18:
		ws = WORD_LENGTH_18;
		break;
	case 20:
		ws = WORD_LENGTH_20;
		break;
	case 24:
		ws = WORD_LENGTH_24;
		break;
	default:
		DRM_ERROR("dp not support sample_width %d\n", audio_info.sample_width);
		goto out;
	}

	phytium_writel_reg(priv, ((fs&ORIG_FREQ_MASK)<<ORIG_FREQ_SHIFT)
			   | ((ws&WORD_LENGTH_MASK) << WORD_LENGTH_SHIFT),
			   group_offset, PHYTIUM_DP_SEC_CS_LENGTH_ORIG_FREQ);
	phytium_writel_reg(priv, (fs_accurac&SAMPLING_FREQ_MASK) << SAMPLING_FREQ_SHIFT,
			   group_offset, PHYTIUM_DP_SEC_CS_FREQ_CLOCK_ACCURACY);

	n_m = phytium_dp_audio_get_n_m(phytium_dp->link_rate, audio_info.sample_rate);
	if (n_m == NULL) {
		DRM_NOTE("can not get n_m for link_rate(%d) and sample_rate(%d)\n",
			       phytium_dp->link_rate, audio_info.sample_rate);
		phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_SEC_MAUD);
		phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_SEC_NAUD);

	} else {
		phytium_writel_reg(priv, n_m->m, group_offset, PHYTIUM_DP_SEC_MAUD);
		phytium_writel_reg(priv, n_m->n, group_offset, PHYTIUM_DP_SEC_NAUD);
	}
	phytium_writel_reg(priv, SECONDARY_STREAM_ENABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	phytium_dp->audio_info = audio_info;

	return 0;

out:
	phytium_writel_reg(priv, SECONDARY_STREAM_DISABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);

	return ret;
}

void phytium_dp_hw_disable_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,
			   group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
}

bool phytium_dp_hw_video_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, config;
	uint32_t group_offset = priv->dp_reg_base[port];

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
	return config ? true : false;
}

void phytium_dp_hw_enable_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SST_MST_SOURCE_0_ENABLE,
			   group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
}

void phytium_dp_hw_config_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	unsigned long link_bw, date_rate = 0;
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	unsigned char tu_size = 64;
	unsigned long data_per_tu = 0;
	int symbols_per_tu, frac_symbols_per_tu, symbol_count, udc, value;

	/* cal M/N and tu_size */
	phytium_writel_reg(priv, phytium_dp->mode.crtc_clock/10, group_offset, PHYTIUM_DP_M_VID);
	phytium_writel_reg(priv, phytium_dp->link_rate/10, group_offset, PHYTIUM_DP_N_VID);
	link_bw = phytium_dp->link_rate * phytium_dp->link_lane_count;
	date_rate = (phytium_dp->mode.crtc_clock * display_info->bpc * 3)/8;

	/* mul 10 for register setting */
	data_per_tu = 10*tu_size * date_rate/link_bw;
	symbols_per_tu = (data_per_tu/10)&0xff;
	if (symbols_per_tu == 63)
		frac_symbols_per_tu = 0;
	else
		frac_symbols_per_tu = (data_per_tu%10*16/10) & 0xf;
	phytium_writel_reg(priv, frac_symbols_per_tu<<24 | symbols_per_tu<<16 | tu_size,
			   group_offset, PHYTIUM_DP_TRANSFER_UNIT_SIZE);

	symbol_count = (phytium_dp->mode.crtc_hdisplay*display_info->bpc*3 + 7)/8;
	udc = (symbol_count + phytium_dp->link_lane_count - 1)/phytium_dp->link_lane_count;
	phytium_writel_reg(priv, udc, group_offset, PHYTIUM_DP_DATA_COUNT);

	/* config main stream attributes */
	phytium_writel_reg(priv, phytium_dp->mode.crtc_htotal,
			   group_offset, PHYTIUM_DP_MAIN_LINK_HTOTAL);
	phytium_writel_reg(priv, phytium_dp->mode.crtc_hdisplay,
			   group_offset, PHYTIUM_DP_MAIN_LINK_HRES);
	phytium_writel_reg(priv,
			   phytium_dp->mode.crtc_hsync_end - phytium_dp->mode.crtc_hsync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_HSWIDTH);
	phytium_writel_reg(priv, phytium_dp->mode.crtc_htotal - phytium_dp->mode.crtc_hsync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_HSTART);
	phytium_writel_reg(priv, phytium_dp->mode.crtc_vtotal,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VTOTAL);
	phytium_writel_reg(priv, phytium_dp->mode.crtc_vdisplay,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VRES);
	phytium_writel_reg(priv,
			   phytium_dp->mode.crtc_vsync_end - phytium_dp->mode.crtc_vsync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VSWIDTH);
	phytium_writel_reg(priv, phytium_dp->mode.crtc_vtotal - phytium_dp->mode.crtc_vsync_start,
			   group_offset, PHYTIUM_DP_MAIN_LINK_VSTART);

	value = 0;
	if (phytium_dp->mode.flags & DRM_MODE_FLAG_PHSYNC)
		value = value & (~HSYNC_POLARITY_LOW);
	else
		value = value | HSYNC_POLARITY_LOW;

	if (phytium_dp->mode.flags & DRM_MODE_FLAG_PVSYNC)
		value = value & (~VSYNC_POLARITY_LOW);
	else
		value = value | VSYNC_POLARITY_LOW;
	phytium_writel_reg(priv, value, group_offset, PHYTIUM_DP_MAIN_LINK_POLARITY);

	switch (display_info->bpc) {
	case 10:
		value = (MISC0_BIT_DEPTH_10BIT << MISC0_BIT_DEPTH_OFFSET);
		break;
	case 6:
		value = (MISC0_BIT_DEPTH_6BIT << MISC0_BIT_DEPTH_OFFSET);
		break;
	default:
		value = (MISC0_BIT_DEPTH_8BIT << MISC0_BIT_DEPTH_OFFSET);
		break;
	}
	value |= (MISC0_COMPONENT_FORMAT_RGB << MISC0_COMPONENT_FORMAT_SHIFT)
		| MISC0_SYNCHRONOUS_CLOCK;
	phytium_writel_reg(priv, value, group_offset, PHYTIUM_DP_MAIN_LINK_MISC0);
	phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_MAIN_LINK_MISC1);

	value = USER_ODDEVEN_POLARITY_HIGH | USER_DATA_ENABLE_POLARITY_HIGH;
	if (phytium_dp->mode.flags & DRM_MODE_FLAG_PHSYNC)
		value = value | USER_HSYNC_POLARITY_HIGH;
	else
		value = value & (~USER_HSYNC_POLARITY_HIGH);
	if (phytium_dp->mode.flags & DRM_MODE_FLAG_PVSYNC)
		value = value | USER_VSYNC_POLARITY_HIGH;
	else
		value = value & (~USER_VSYNC_POLARITY_HIGH);
	phytium_writel_reg(priv, value, group_offset, PHYTIUM_DP_USER_SYNC_POLARITY);
}

void phytium_dp_hw_disable_output(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_DISABLE,
			   group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
}

void phytium_dp_hw_enable_output(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_ENABLE,
			   group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
}

void phytium_dp_hw_enable_input_source(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, VIRTUAL_SOURCE_0_ENABLE,
			   group_offset, PHYTIUM_INPUT_SOURCE_ENABLE);
}

void phytium_dp_hw_disable_input_source(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;

	phytium_writel_reg(priv, (~VIRTUAL_SOURCE_0_ENABLE)&VIRTUAL_SOURCE_0_ENABLE_MASK,
			   priv->dp_reg_base[port], PHYTIUM_INPUT_SOURCE_ENABLE);
}

bool phytium_dp_hw_output_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	int config = 0;

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	return config ? true : false;
}

static void phytium_dp_hw_get_hpd_state(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t val = 0, raw_state = 0;
	uint32_t group_offset = priv->dp_reg_base[port];

	val = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_RAW_STATUS);

	/* maybe miss hpd, so used for clear PHYTIUM_DP_INTERRUPT_RAW_STATUS */
	phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
	raw_state = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SINK_HPD_STATE);
	if (val & HPD_EVENT)
		phytium_dp->dp_hpd_state.hpd_event_state = true;

	if (val & HPD_IRQ)
		phytium_dp->dp_hpd_state.hpd_irq_state = true;

	if (raw_state & HPD_CONNECT)
		phytium_dp->dp_hpd_state.hpd_raw_state = true;
	else
		phytium_dp->dp_hpd_state.hpd_raw_state = false;
}

void phytium_dp_hw_hpd_irq_setup(struct phytium_dp_device *phytium_dp, bool enable)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_dp->dp_hpd_state.hpd_irq_enable = enable;
	if (enable)
		phytium_writel_reg(priv, HPD_OTHER_MASK, group_offset, PHYTIUM_DP_INTERRUPT_MASK);
	else
		phytium_writel_reg(priv, HPD_IRQ_MASK|HPD_EVENT_MASK|HPD_OTHER_MASK,
				   group_offset, PHYTIUM_DP_INTERRUPT_MASK);
}

int phytium_dp_hw_init(struct phytium_dp_device *phytium_dp)
{
	int ret = 0;
	uint8_t count = 0;

	phytium_dp->source_rates = phytium_rate;
	phytium_dp->num_source_rates = num_source_rates;
	count = phytium_dp->funcs->dp_hw_get_source_lane_count(phytium_dp);
	phytium_dp->source_max_lane_count = count;

	ret = phytium_dp->funcs->dp_hw_reset(phytium_dp);
	if (ret)
		goto out;
	ret = phytium_dp->funcs->dp_hw_init_phy(phytium_dp);
	if (ret)
		goto out;

	phytium_dp->fast_train_support = false;
	phytium_dp->hw_spread_enable = phytium_dp->funcs->dp_hw_spread_is_enable(phytium_dp);

out:
	return ret;
}

static int phytium_dp_dpcd_get_tp_link(struct phytium_dp_device *phytium_dp,
						    uint8_t *test_lane_count,
						    uint32_t *test_link_rate)
{
	uint8_t test_link_bw;
	int ret;

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_TEST_LANE_COUNT,
				   test_lane_count);
	if (ret <= 0) {
		DRM_DEBUG_KMS("test pattern Lane count read failed(%d)\n", ret);
		goto failed;
	}

	ret = drm_dp_dpcd_readb(&phytium_dp->aux, DP_TEST_LINK_RATE,
				   &test_link_bw);
	if (ret <= 0) {
		DRM_DEBUG_KMS("test pattern link rate read failed(%d)\n", ret);
		goto failed;
	}
	*test_link_rate = drm_dp_bw_code_to_link_rate(test_link_bw);

	return 0;
failed:
	return ret;
}

static int phytium_dp_dpcd_set_link(struct phytium_dp_device *phytium_dp,
					       uint8_t lane_count, uint32_t link_rate)
{
	uint8_t link_config[2];
	int ret = 0;

	link_config[0] = drm_dp_link_rate_to_bw_code(link_rate);
	link_config[1] = lane_count;
	if (drm_dp_enhanced_frame_cap(phytium_dp->dpcd)) {
		link_config[1] |= DP_LANE_COUNT_ENHANCED_FRAME_EN;
	}
	ret = drm_dp_dpcd_write(&phytium_dp->aux, DP_LINK_BW_SET, link_config, 2);
	if (ret < 0) {
		DRM_NOTE("write dpcd DP_LINK_BW_SET fail: ret:%d\n", ret);
		goto failed;
	}

	if (phytium_dp->hw_spread_enable)
		link_config[0] = DP_SPREAD_AMP_0_5;
	else
		link_config[0] = 0;
	link_config[1] = DP_SET_ANSI_8B10B;
	ret = drm_dp_dpcd_write(&phytium_dp->aux, DP_DOWNSPREAD_CTRL, link_config, 2);
	if (ret < 0) {
		DRM_ERROR("write DP_DOWNSPREAD_CTRL fail: ret:%d\n", ret);
		goto failed;
	}

	return 0;
failed:
	return ret;
}

static int phytium_dp_dpcd_set_test_pattern(struct phytium_dp_device *phytium_dp,
							   uint8_t test_pattern)
{
	unsigned char value;
	int ret;

	if (phytium_dp_coding_8b10b_need_enable(test_pattern))
		value = DP_SET_ANSI_8B10B;
	else
		value = 0;
	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_MAIN_LINK_CHANNEL_CODING_SET, value);
	if (ret < 0) {
		DRM_ERROR("write DP_MAIN_LINK_CHANNEL_CODING_SET fail: ret:%d\n", ret);
		goto failed;
	}

	if (phytium_dp_scrambled_need_enable(test_pattern))
		value = DP_TRAINING_PATTERN_DISABLE;
	else
		value = (DP_TRAINING_PATTERN_DISABLE | DP_LINK_SCRAMBLING_DISABLE);

	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_TRAINING_PATTERN_SET, value);
	if (ret < 0) {
		DRM_ERROR("write DP_TRAINING_PATTERN_SET fail: ret:%d\n", ret);
		goto failed;
	}

	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_LINK_QUAL_LANE0_SET, test_pattern);
	if (ret < 0) {
		DRM_ERROR("write DP_TRAINING_PATTERN_SET fail: ret:%d\n", ret);
		goto failed;
	}

	return 0;
failed:
	return ret;
}

static int phytium_dp_dpcd_set_train_pattern(struct phytium_dp_device *phytium_dp,
						     uint8_t train_pattern)
{
	uint8_t value;
	int ret;

	/* Scrambling is disabled for TPS1/2/3 and enabled for TPS4 */
	if (train_pattern == DP_TRAINING_PATTERN_4 || train_pattern == DP_TRAINING_PATTERN_DISABLE)
		value = train_pattern;
	else
		value = (train_pattern | DP_LINK_SCRAMBLING_DISABLE);

	ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_TRAINING_PATTERN_SET, value);
	if (ret < 0) {
		DRM_NOTE("write DP_TRAINING_PATTERN_SET fail: ret:%d\n", ret);
		goto failed;
	}

	return 0;
failed:
	return ret;
}

static int
phytium_dp_dpcd_set_lane_setting(struct phytium_dp_device *phytium_dp, uint8_t *train_set)
{
	int ret = 0;

	ret =  drm_dp_dpcd_write(&phytium_dp->aux, DP_TRAINING_LANE0_SET,
				 phytium_dp->train_set, 4);
	if (ret < 0) {
		DRM_ERROR("write DP_TRAINING_LANE0_SET fail: ret:%d\n", ret);
		return ret;
	}

	return 0;
}

static int
phytium_dp_dpcd_get_adjust_request(struct phytium_dp_device *phytium_dp, uint8_t lane_count)
{
	int ret = 0;
	uint8_t link_status[DP_LINK_STATUS_SIZE];

	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_LANE0_1_STATUS,
		       link_status, DP_LINK_STATUS_SIZE);
	if (ret < 0) {
		DRM_ERROR("failed to get link status(DP_LANE0_1_STATUS)\n");
		goto failed;
	}
	phytium_get_adjust_train(phytium_dp, link_status, lane_count);

	return 0;
failed:
	return ret;
}

void phytium_dp_dpcd_sink_dpms(struct phytium_dp_device *phytium_dp, int mode)
{
	int ret, i;

	if (phytium_dp->dpcd[DP_DPCD_REV] < 0x11)
		return;
	if (mode != DRM_MODE_DPMS_ON) {
		ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_SET_POWER, DP_SET_POWER_D3);
	} else {
		for (i = 0; i < 3; i++) {
			ret = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_SET_POWER, DP_SET_POWER_D0);
			if (ret == 1)
				break;
			msleep(20);
		}
	}

	if (ret != 1)
		DRM_DEBUG_KMS("failed to %s sink power state\n",
			       mode == DRM_MODE_DPMS_ON ? "enable" : "disable");
}

static bool phytium_dp_link_training_clock_recovery(struct phytium_dp_device *phytium_dp)
{
	int ret;
	unsigned char voltage, max_vswing_tries;
	int voltage_tries;

	/* clear the test pattern */
	phytium_dp_hw_set_test_pattern(phytium_dp, phytium_dp->link_lane_count,
				       PHYTIUM_PHY_TP_NONE, NULL, 0);

	/* config source and sink's link rate and lane count */
	phytium_dp_hw_set_link(phytium_dp, phytium_dp->link_lane_count, phytium_dp->link_rate);
	ret = phytium_dp_dpcd_set_link(phytium_dp, phytium_dp->link_lane_count,
				       phytium_dp->link_rate);
	if (ret < 0) {
		DRM_NOTE("phytium_dp_dpcd_set_link failed(ret=%d)\n", ret);
		return false;
	}

	/* config source's voltage swing and pre-emphasis(103-106) */
	memset(phytium_dp->train_set, 0, sizeof(phytium_dp->train_set));
	phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->link_rate,
				       phytium_dp->train_set[0]);

	/* config train pattern */
	phytium_dp_hw_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_1);
	ret = phytium_dp_dpcd_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_1);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_train_pattern fail: ret:%d\n", ret);
		return false;
	}

	/* config sink's voltage swing and pre-emphasis(103-106) */
	ret = phytium_dp_dpcd_set_lane_setting(phytium_dp, phytium_dp->train_set);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_lane_setting fail: ret:%d\n", ret);
		return false;
	}

	voltage_tries = 1;
	max_vswing_tries = 0;
	for (;;) {
		unsigned char link_status[DP_LINK_STATUS_SIZE];
		drm_dp_link_train_clock_recovery_delay(phytium_dp->dpcd);
		/* get link status 0x202-0x207 */
		ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_LANE0_1_STATUS,
				       link_status, DP_LINK_STATUS_SIZE);
		if (ret < 0) {
			DRM_ERROR("failed to get link status(DP_LANE0_1_STATUS)\n");
			return false;
		}

		if (drm_dp_clock_recovery_ok(link_status, phytium_dp->link_lane_count)) {
			DRM_DEBUG_KMS("clock revorery ok\n");
			return true;
		}

		if (voltage_tries == 5) {
			DRM_DEBUG_KMS("Same voltage tried 5 times\n");
			return false;
		}

		if (max_vswing_tries == 1) {
			DRM_DEBUG_KMS("Max Voltage Swing reached\n");
			return false;
		}

		voltage = phytium_dp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK;

		/* config source and sink's voltage swing and pre-emphasis(103-106) */
		phytium_get_adjust_train(phytium_dp, link_status, phytium_dp->link_lane_count);
		phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->link_rate,
					       phytium_dp->train_set[0]);
		ret = phytium_dp_dpcd_set_lane_setting(phytium_dp, phytium_dp->train_set);
		if (ret < 0) {
			DRM_ERROR("phytium_dp_dpcd_set_lane_setting fail: ret:%d\n", ret);
			return false;
		}

		if ((phytium_dp->train_set[0] & DP_TRAIN_VOLTAGE_SWING_MASK) == voltage)
			++voltage_tries;
		else
			voltage_tries = 1;

		if (phytium_dp->train_set[0] & DP_TRAIN_MAX_SWING_REACHED)
			++max_vswing_tries;

		DRM_DEBUG_KMS("try train_set:0x%x voltage_tries:%d max_vswing_tries:%d\n",
			       phytium_dp->train_set[0], voltage_tries, max_vswing_tries);
	}
}

static unsigned int phytium_dp_get_training_pattern(struct phytium_dp_device *phytium_dp)
{
	bool sink_tps3, sink_tps4;

	sink_tps4 = drm_dp_tps4_supported(phytium_dp->dpcd);
	if (sink_tps4)
		return DP_TRAINING_PATTERN_4;
	else if (phytium_dp->link_rate == 810000)
		DRM_DEBUG_KMS("8.1 Gbps link rate without sink TPS4 support\n");

	sink_tps3 = drm_dp_tps3_supported(phytium_dp->dpcd);
	if (sink_tps3)
		return DP_TRAINING_PATTERN_3;
	else if (phytium_dp->link_rate >= 540000)
		DRM_DEBUG_KMS(">=5.4/6.48 Gbps link rate without sink TPS3 support\n");

	return DP_TRAINING_PATTERN_2;
}

static bool phytium_dp_link_training_channel_equalization(struct phytium_dp_device *phytium_dp)
{
	unsigned int training_pattern;
	int tries, ret;
	unsigned char link_status[DP_LINK_STATUS_SIZE];
	bool channel_eq = false;

	/* config source and sink's voltage swing and pre-emphasis(103-106), from clock recovery */
	phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->link_rate,
				       phytium_dp->train_set[0]);
	ret = phytium_dp_dpcd_set_lane_setting(phytium_dp, phytium_dp->train_set);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_lane_setting fail: ret:%d\n", ret);
		return channel_eq;
	}

	/* config source and sink's train_pattern x */
	training_pattern = phytium_dp_get_training_pattern(phytium_dp);
	phytium_dp_hw_set_train_pattern(phytium_dp, training_pattern);
	ret = phytium_dp_dpcd_set_train_pattern(phytium_dp, training_pattern);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_train_pattern fail: ret:%d\n", ret);
		return channel_eq;
	}

	for (tries = 0; tries < 5; tries++) {
		drm_dp_link_train_channel_eq_delay(phytium_dp->dpcd);

		/* get link status 0x202-0x207 */
		ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_LANE0_1_STATUS,
				       link_status, DP_LINK_STATUS_SIZE);
		if (ret < 0) {
			DRM_ERROR("failed to get link status(DP_LANE0_1_STATUS)\n");
			break;
		}

		/* Make sure clock is still ok */
		if (!drm_dp_clock_recovery_ok(link_status, phytium_dp->link_lane_count)) {
			DRM_DEBUG_KMS("CR check failed, cannot continue channel equalization\n");
			break;
		}

		if (drm_dp_channel_eq_ok(link_status, phytium_dp->link_lane_count)) {
			channel_eq = true;
			DRM_DEBUG_KMS("Channel EQ done. DP Training successful\n");
			break;
		}

		/* config source and sink's voltage swing and pre-emphasis(103-106) */
		phytium_get_adjust_train(phytium_dp, link_status, phytium_dp->link_lane_count);
		phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->link_rate,
					       phytium_dp->train_set[0]);
		ret = phytium_dp_dpcd_set_lane_setting(phytium_dp, phytium_dp->train_set);
		if (ret < 0) {
			DRM_ERROR("phytium_dp_dpcd_set_lane_setting fail: ret:%d\n", ret);
			break;
		}
	}

	/* Try 5 times, else fail and try at lower BW */
	if (tries == 5)
		DRM_DEBUG_KMS("Channel equalization failed 5 times\n");

	return channel_eq;
}

static void phytium_dp_train_retry_work_fn(struct work_struct *work)
{
	struct phytium_dp_device *phytium_dp = train_retry_to_dp_device(work);
	struct drm_connector *connector;

	connector = &phytium_dp->connector;
	DRM_DEBUG_KMS("[CONNECTOR:%d:%s]\n", connector->base.id, connector->name);
	mutex_lock(&connector->dev->mode_config.mutex);
	drm_connector_set_link_status_property(connector, DRM_MODE_LINK_STATUS_BAD);
	mutex_unlock(&connector->dev->mode_config.mutex);
	drm_kms_helper_hotplug_event(connector->dev);
}

/* return index of rate in rates array, or -1 if not found */
static int phytium_dp_rate_index(const int *rates, int len, int rate)
{
	int i;

	for (i = 0; i < len; i++)
		if (rate == rates[i])
			return i;

	return -1;
}

int phytium_dp_get_link_train_fallback_values(struct phytium_dp_device *phytium_dp)
{
	int index, ret = 0;

	if (phytium_dp->is_edp) {
		phytium_dp->train_retry_count++;
		DRM_INFO("Retrying Link training for eDP(%d) with same parameters\n",
			  phytium_dp->port);
		goto out;
	} else {
		index = phytium_dp_rate_index(phytium_dp->common_rates,
				    phytium_dp->num_common_rates,
				    phytium_dp->link_rate);
		if (index > 0) {
			phytium_dp->link_rate = phytium_dp->common_rates[index - 1];
		} else if (phytium_dp->link_lane_count > 1) {
			phytium_dp->link_rate = phytium_dp->max_link_rate;
			phytium_dp->link_lane_count = phytium_dp->link_lane_count >> 1;
		} else {
			phytium_dp->train_retry_count++;
			phytium_dp->link_rate = phytium_dp->max_link_rate;
			phytium_dp->link_lane_count = phytium_dp->max_link_lane_count;
			DRM_INFO("Retrying Link training for DP(%d) with maximal parameters\n",
				  phytium_dp->port);
			ret = -1;
		}
	}

out:
	return ret;
}

static int
phytium_dp_stop_link_train(struct phytium_dp_device *phytium_dp)
{
	int ret;

	/* config source and sink's train_pattern x: DP_TRAINING_PATTERN_DISABLE */
	phytium_dp_hw_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_DISABLE);

	ret = phytium_dp_dpcd_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_DISABLE);
	if (ret < 0) {
		DRM_NOTE("phytium_dp_dpcd_set_train_pattern fail: ret:%d\n", ret);
		return ret;
	}

	return 0;
}

int phytium_dp_start_link_train(struct phytium_dp_device *phytium_dp)
{
	int ret = 0;

	phytium_dp_hw_disable_output(phytium_dp);
	phytium_dp_hw_disable_input_source(phytium_dp);
	phytium_dp_hw_disable_video(phytium_dp);
	phytium_dp_hw_enable_input_source(phytium_dp);
	phytium_dp_hw_enable_output(phytium_dp);
	phytium_dp_dpcd_sink_dpms(phytium_dp, DRM_MODE_DPMS_OFF);
	phytium_dp_dpcd_sink_dpms(phytium_dp, DRM_MODE_DPMS_ON);

	if (!phytium_dp_link_training_clock_recovery(phytium_dp))
		goto failure_handling;

	if (!phytium_dp_link_training_channel_equalization(phytium_dp))
		goto failure_handling;

	ret = phytium_dp_stop_link_train(phytium_dp);
	if (ret < 0) {
		DRM_NOTE("phytium_dp_stop_link_train failed: ret = %d\n", ret);
		goto out;
	}

	if (phytium_dp->trigger_train_fail) {
		phytium_dp->trigger_train_fail--;
		goto failure_handling;
	}
	phytium_dp->train_retry_count = 0;

	DRM_DEBUG_KMS("[CONNECTOR:%d:%s] Link Training Pass at Link Rate = %d, Lane count = %d\n",
		      phytium_dp->connector.base.id,
		      phytium_dp->connector.name, phytium_dp->link_rate,
		      phytium_dp->link_lane_count);

	return 0;

failure_handling:
	DRM_INFO("[CONNECTOR:%d:%s] Link Training failed at Link Rate = %d, Lane count = %d",
	      phytium_dp->connector.base.id,
	      phytium_dp->connector.name,
	      phytium_dp->link_rate, phytium_dp->link_lane_count);

	ret = phytium_dp_stop_link_train(phytium_dp);
	if (ret < 0) {
		DRM_NOTE("phytium_dp_stop_link_train failed: ret = %d\n", ret);
		goto out;
	}

	phytium_dp_get_link_train_fallback_values(phytium_dp);
	if (phytium_dp->train_retry_count < 5)
		schedule_work(&phytium_dp->train_retry_work);
	else
		DRM_ERROR("DP(%d) Link Training Unsuccessful, and stop Training\n",
			   phytium_dp->port);

out:
	return -1;
}

static bool phytium_dp_needs_link_retrain(struct phytium_dp_device *phytium_dp)
{
	unsigned char link_status[DP_LINK_STATUS_SIZE];
	int ret = 0;

	/* get link status 0x202-0x207 */
	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_LANE0_1_STATUS,
			       link_status, DP_LINK_STATUS_SIZE);
	if (ret < 0) {
		DRM_ERROR("failed to get link status(DP_LANE0_1_STATUS)\n");
		return true;
	}

	if ((phytium_dp->link_rate == 0) ||  (phytium_dp->link_lane_count == 0)) {
		DRM_DEBUG_KMS("link_rate(%d) or lane_count(%d) is invalid\n",
			       phytium_dp->link_rate, phytium_dp->link_lane_count);
		return true;
	}

	/* Make sure clock is still ok */
	if (!drm_dp_clock_recovery_ok(link_status, phytium_dp->link_lane_count)) {
		DRM_DEBUG_KMS("Clock recovery check failed\n");
		return true;
	}

	if (!drm_dp_channel_eq_ok(link_status, phytium_dp->link_lane_count)) {
		DRM_DEBUG_KMS("Channel EQ check failed\n");
		return true;
	}

	if (!phytium_dp_hw_output_is_enable(phytium_dp)) {
		DRM_DEBUG_KMS("check DP output enable failed\n");
		return true;
	}
	return false;
}

static bool
phytium_dp_get_sink_irq(struct phytium_dp_device *phytium_dp, u8 *sink_irq_vector)
{
	return drm_dp_dpcd_readb(&phytium_dp->aux, DP_DEVICE_SERVICE_IRQ_VECTOR,
				 sink_irq_vector) == 1;
}

static uint8_t phytium_dp_autotest_phy_pattern(struct phytium_dp_device *phytium_dp)
{
	union phytium_phy_tp phytium_phy_tp;
	int ret;
	unsigned char test_80_bit_pattern[
		(DP_TEST_80BIT_CUSTOM_PATTERN_79_72 -
		DP_TEST_80BIT_CUSTOM_PATTERN_7_0)+1] = {0};
	unsigned char test_pattern;
	unsigned int offset;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	offset = DP_TEST_PHY_PATTERN;
#else
	offset = DP_PHY_TEST_PATTERN;
#endif

	ret = drm_dp_dpcd_read(&phytium_dp->aux, offset,
				   &phytium_phy_tp.raw,
				   sizeof(phytium_phy_tp));
	if (ret <= 0) {
		DRM_DEBUG_KMS("Could not read DP_TEST_PHY_PATTERN\n");
		goto failed;
	}

	test_pattern = phytium_phy_tp.bits.PATTERN;

	if (test_pattern == PHYTIUM_PHY_TP_80BIT_CUSTOM) {
		ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_TEST_80BIT_CUSTOM_PATTERN_7_0,
					   test_80_bit_pattern,
					   sizeof(test_80_bit_pattern));
		if (ret <= 0) {
			DRM_DEBUG_KMS("Could not read DP_TEST_PHY_PATTERN\n");
			goto failed;
		}
	}

	/* config source and sink's link rate and link count */
	ret = phytium_dp_dpcd_get_tp_link(phytium_dp, &phytium_dp->compliance.test_lane_count,
				    &phytium_dp->compliance.test_link_rate);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_get_tp_link fail: ret:%d\n", ret);
		goto failed;
	}

	phytium_dp_hw_set_link(phytium_dp, phytium_dp->compliance.test_lane_count,
			       phytium_dp->compliance.test_link_rate);
	ret = phytium_dp_dpcd_set_link(phytium_dp, phytium_dp->compliance.test_lane_count,
				       phytium_dp->compliance.test_link_rate);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_link fail: ret:%d\n", ret);
		goto failed_dpcd_set_link;
	}

	/* config source and sink's lane setting: voltage swing and pre-emphasis */
	ret = phytium_dp_dpcd_get_adjust_request(phytium_dp,
						 phytium_dp->compliance.test_lane_count);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_get_adjust_request fail: ret:%d\n", ret);
		goto failed_dpcd_get_adjust_request;
	}
	phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->compliance.test_link_rate,
				       phytium_dp->train_set[0]);
	ret = phytium_dp_dpcd_set_lane_setting(phytium_dp, phytium_dp->train_set);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_lane_setting fail: ret:%d\n", ret);
		goto failed_dpcd_set_lane_setting;
	}

	/* config test pattern */
	phytium_dp_hw_set_test_pattern(phytium_dp, phytium_dp->compliance.test_lane_count,
				       test_pattern, test_80_bit_pattern,
				       sizeof(test_80_bit_pattern));
	ret = phytium_dp_dpcd_set_test_pattern(phytium_dp, test_pattern);
	if (ret < 0) {
		DRM_ERROR("phytium_dp_dpcd_set_test_pattern fail: ret:%d\n", ret);
		goto failed_dpcd_set_tp;
	}

	return DP_TEST_ACK;

failed_dpcd_set_tp:
	phytium_dp_hw_set_test_pattern(phytium_dp, phytium_dp->compliance.test_lane_count,
				       PHYTIUM_PHY_TP_NONE, test_80_bit_pattern,
				       sizeof(test_80_bit_pattern));
failed_dpcd_set_link:
failed_dpcd_set_lane_setting:
failed_dpcd_get_adjust_request:
failed:
	return DP_TEST_NAK;
}

static void phytium_dp_handle_test_request(struct phytium_dp_device *phytium_dp)
{
	uint8_t response = DP_TEST_NAK;
	uint8_t request = 0;
	int status;

	status = drm_dp_dpcd_readb(&phytium_dp->aux, DP_TEST_REQUEST, &request);
	if (status <= 0) {
		DRM_DEBUG_KMS("Could not read test request from sink\n");
		goto update_status;
	}

	switch (request) {
	case DP_TEST_LINK_TRAINING:
	case DP_TEST_LINK_VIDEO_PATTERN:
	case DP_TEST_LINK_EDID_READ:
		DRM_DEBUG_KMS("Not support test request '%02x'\n", request);
		response = DP_TEST_NAK;
		break;
	case DP_TEST_LINK_PHY_TEST_PATTERN:
		DRM_DEBUG_KMS("PHY_PATTERN test requested\n");
		response = phytium_dp_autotest_phy_pattern(phytium_dp);
		break;
	default:
		DRM_DEBUG_KMS("Invalid test request '%02x'\n", request);
		break;
	}

update_status:
	status = drm_dp_dpcd_writeb(&phytium_dp->aux, DP_TEST_RESPONSE, response);
	if (status <= 0)
		DRM_DEBUG_KMS("Could not write test response to sink\n");

}

static int phytium_dp_long_pulse(struct drm_connector *connector, bool hpd_raw_state)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	enum drm_connector_status status = connector->status;
	bool video_enable = false;
	uint32_t index = 0;

	if (phytium_dp->is_edp)
		status = connector_status_connected;
	else if (hpd_raw_state) {
		if (!phytium_dp_needs_link_retrain(phytium_dp)) {
			status = connector_status_connected;
			goto out;
		}
	} else {
		status = connector_status_disconnected;
		goto out;
	}

	if (!phytium_dp->is_edp) {
		status = phytium_dp_detect_dpcd(phytium_dp);
		if (status == connector_status_disconnected)
			goto out;

		index = phytium_dp->num_common_rates-1;
		phytium_dp->max_link_rate = phytium_dp->common_rates[index];
		phytium_dp->max_link_lane_count = phytium_dp->common_max_lane_count;
		phytium_dp->link_rate = phytium_dp->max_link_rate;
		phytium_dp->link_lane_count = phytium_dp->max_link_lane_count;
		DRM_DEBUG_KMS("common_max_lane_count: %d, common_max_rate:%d\n",
			       phytium_dp->max_link_lane_count, phytium_dp->max_link_rate);

		video_enable = phytium_dp_hw_video_is_enable(phytium_dp);
		phytium_dp_start_link_train(phytium_dp);

		if (video_enable) {
			mdelay(2);
			phytium_dp_hw_enable_video(phytium_dp);
		}
	}

out:
	return status;
}

static int phytium_dp_short_pulse(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	enum drm_connector_status status = connector->status;
	u8 sink_irq_vector = 0;
	bool video_enable = false;

	/* handle the test pattern */
	if (phytium_dp_get_sink_irq(phytium_dp, &sink_irq_vector) &&
	    sink_irq_vector != 0) {
		drm_dp_dpcd_writeb(&phytium_dp->aux,
				   DP_DEVICE_SERVICE_IRQ_VECTOR,
				   sink_irq_vector);
		if (sink_irq_vector & DP_AUTOMATED_TEST_REQUEST)
			phytium_dp_handle_test_request(phytium_dp);
		if (sink_irq_vector & (DP_CP_IRQ | DP_SINK_SPECIFIC_IRQ))
			DRM_DEBUG_DRIVER("CP or sink specific irq unhandled\n");
	}
	if (!phytium_dp_needs_link_retrain(phytium_dp)) {
		status = connector_status_connected;
		goto out;
	}

	video_enable = phytium_dp_hw_video_is_enable(phytium_dp);
	phytium_dp_start_link_train(phytium_dp);
	if (video_enable) {
		mdelay(2);
		phytium_dp_hw_enable_video(phytium_dp);
	}

out:
	return status;
}

void phytium_dp_hpd_poll_handler(struct phytium_display_private *priv)
{
	struct drm_device *dev = priv->dev;
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *connector;
	enum drm_connector_status old_status;
	bool changed = false;

	mutex_lock(&dev->mode_config.mutex);
	DRM_DEBUG_KMS("running encoder hotplug poll functions\n");
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (connector->force)
			continue;
		old_status = connector->status;
		connector->status = drm_helper_probe_detect(connector, NULL, false);
		if (old_status != connector->status) {
			const char *old, *new;

			old = drm_get_connector_status_name(old_status);
			new = drm_get_connector_status_name(connector->status);
			DRM_DEBUG_KMS("[CONNECTOR:%d:%s] status updated from %s to %s\n",
					connector->base.id,
					connector->name,
					old, new);
			changed = true;
		}
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	if (changed)
		drm_kms_helper_hotplug_event(dev);
}

void phytium_dp_hpd_irq_setup(struct drm_device *dev, bool enable)
{
	struct phytium_dp_device *phytium_dp;
	struct drm_encoder *encoder;
	struct phytium_display_private *priv = dev->dev_private;
	bool handler = false;
	bool hpd_raw_state_old = false;

	/* We might have missed any hotplugs that happened, so polling and handler */
	if (enable) {
		spin_lock_irq(&priv->hotplug_irq_lock);

		drm_for_each_encoder(encoder, dev) {
			phytium_dp = encoder_to_dp_device(encoder);
			if (!phytium_dp->dp_hpd_state.hpd_irq_enable) {
				hpd_raw_state_old = phytium_dp->dp_hpd_state.hpd_raw_state;
				phytium_dp_hw_get_hpd_state(phytium_dp);
				if (phytium_dp->dp_hpd_state.hpd_event_state
				|| phytium_dp->dp_hpd_state.hpd_irq_state
				|| (hpd_raw_state_old != phytium_dp->dp_hpd_state.hpd_raw_state)) {
					handler = true;
				}
			}
		}
		spin_unlock_irq(&priv->hotplug_irq_lock);
		if (handler)
			phytium_dp_hpd_poll_handler(priv);
	}

	drm_for_each_encoder(encoder, dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		phytium_dp_hw_hpd_irq_setup(phytium_dp, enable);
	}
}

void phytium_dp_hpd_work_func(struct work_struct *work)
{
	struct phytium_display_private *priv =
		container_of(work, struct phytium_display_private, hotplug_work);
	struct drm_device *dev = priv->dev;
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *connector;
	enum drm_connector_status old_status;
	bool changed = false;

	mutex_lock(&dev->mode_config.mutex);
	DRM_DEBUG_KMS("running encoder hotplug work functions\n");
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) {
		if (connector->force)
			continue;
		old_status = connector->status;
		connector->status = drm_helper_probe_detect(connector, NULL, false);
		if (old_status != connector->status) {
			const char *old, *new;

			old = drm_get_connector_status_name(old_status);
			new = drm_get_connector_status_name(connector->status);
			DRM_DEBUG_KMS("[CONNECTOR:%d:%s] status updated from %s to %s\n",
					connector->base.id,
					connector->name,
					old, new);
			changed = true;
		}
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	if (changed)
		drm_kms_helper_hotplug_event(dev);

	phytium_dp_hpd_irq_setup(dev, true);
}

irqreturn_t phytium_dp_hpd_irq_handler(struct phytium_display_private *priv)
{
	struct drm_encoder *encoder = NULL;
	struct phytium_dp_device *phytium_dp = NULL;
	struct drm_device *dev = priv->dev;
	bool handler = false;

	spin_lock(&priv->hotplug_irq_lock);

	drm_for_each_encoder(encoder, dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		if (phytium_dp->dp_hpd_state.hpd_irq_enable) {
			phytium_dp_hw_get_hpd_state(phytium_dp);
			if (phytium_dp->dp_hpd_state.hpd_event_state
			|| phytium_dp->dp_hpd_state.hpd_irq_state) {
				handler = true;
			}
		}
	}
	spin_unlock(&priv->hotplug_irq_lock);

	if (handler) {
		phytium_dp_hpd_irq_setup(dev, false);
		schedule_work(&priv->hotplug_work);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}


static void phytium_dp_fast_link_train_detect(struct phytium_dp_device *phytium_dp)
{
	phytium_dp->fast_train_support = !!(phytium_dp->dpcd[DP_MAX_DOWNSPREAD]
					    & DP_NO_AUX_HANDSHAKE_LINK_TRAINING);
	DRM_DEBUG_KMS("fast link training %s\n",
		       phytium_dp->fast_train_support ? "supported" : "unsupported");
}

bool phytium_dp_fast_link_train(struct phytium_dp_device *phytium_dp)
{
	int ret = 0;
	unsigned int training_pattern;

	/* clear the test pattern */
	phytium_dp_hw_set_test_pattern(phytium_dp, phytium_dp->link_lane_count,
				       PHYTIUM_PHY_TP_NONE, NULL, 0);

	/* config source and sink's link rate and lane count */
	phytium_dp_hw_set_link(phytium_dp, phytium_dp->link_lane_count, phytium_dp->link_rate);

	/* config source and sink's voltage swing and pre-emphasis(103-106) */
	phytium_dp_hw_set_lane_setting(phytium_dp, phytium_dp->link_rate,
				       phytium_dp->train_set[0]);

	/* config train pattern */
	phytium_dp_hw_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_1);
	usleep_range(500, 600);

	training_pattern = phytium_dp_get_training_pattern(phytium_dp);
	phytium_dp_hw_set_train_pattern(phytium_dp, training_pattern);
	usleep_range(500, 600);

	phytium_dp_hw_set_train_pattern(phytium_dp, DP_TRAINING_PATTERN_DISABLE);

	if (dc_fast_training_check) {
		unsigned char link_status[DP_LINK_STATUS_SIZE];

		ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_LANE0_1_STATUS,
				       link_status, DP_LINK_STATUS_SIZE);
		if (ret < 0) {
			DRM_ERROR("failed to get link status(DP_LANE0_1_STATUS)\n");
			return false;
		}

		if (!drm_dp_clock_recovery_ok(link_status, phytium_dp->link_lane_count)) {
			DRM_DEBUG_KMS("check clock recovery failed\n");
			return false;
		}

		if (!drm_dp_channel_eq_ok(link_status, phytium_dp->link_lane_count)) {
			DRM_DEBUG_KMS("check channel equalization failed\n");
			return false;
		}
	}

	return true;
}

static enum drm_connector_status
phytium_connector_detect(struct drm_connector *connector, bool force)
{
	enum drm_connector_status status = connector->status;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	bool hpd_event_state, hpd_irq_state, hpd_raw_state;
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	bool plugged = true;

	spin_lock_irq(&priv->hotplug_irq_lock);
	hpd_event_state = phytium_dp->dp_hpd_state.hpd_event_state;
	hpd_irq_state = phytium_dp->dp_hpd_state.hpd_irq_state;
	hpd_raw_state = phytium_dp->dp_hpd_state.hpd_raw_state;
	phytium_dp->dp_hpd_state.hpd_event_state = false;
	phytium_dp->dp_hpd_state.hpd_irq_state = false;
	spin_unlock_irq(&priv->hotplug_irq_lock);

	if (hpd_event_state)
		status = phytium_dp_long_pulse(connector, hpd_raw_state);

	if (hpd_irq_state)
		status = phytium_dp_short_pulse(connector);

	if (status == connector_status_unknown)
		status = connector_status_disconnected;

	if ((!phytium_dp->is_edp) && (!hpd_raw_state))
		status = connector_status_disconnected;

	if (connector->status != status) {
		if ((status == connector_status_connected) && phytium_dp->has_audio)
			plugged = true;
		else
			plugged = false;

		handle_plugged_change(phytium_dp, plugged);
	}

	return status;
}

static void
phytium_connector_destroy(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	drm_connector_cleanup(connector);
	kfree(phytium_dp);
}

static int
phytium_dp_connector_register(struct drm_connector *connector)
{
	int ret;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	phytium_dp_aux_init(phytium_dp);
	if (phytium_dp->is_edp) {
		phytium_edp_init_connector(phytium_dp);
		ret = phytium_edp_backlight_device_register(phytium_dp);
		if (ret)
			DRM_ERROR("failed to register port(%d) backlight device(ret=%d)\n",
				   phytium_dp->port, ret);
	}

	ret = phytium_debugfs_connector_add(connector);
	if (ret)
		DRM_ERROR("failed to register phytium connector debugfs(ret=%d)\n", ret);

	return 0;
}

static void
phytium_dp_connector_unregister(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);

	if (phytium_dp->is_edp) {
		phytium_edp_backlight_device_unregister(phytium_dp);
		phytium_edp_fini_connector(phytium_dp);
	}
	drm_dp_aux_unregister(&phytium_dp->aux);
}

static const struct drm_connector_funcs phytium_connector_funcs = {
	.dpms			= drm_helper_connector_dpms,
	.detect			= phytium_connector_detect,
	.fill_modes		= drm_helper_probe_single_connector_modes,
	.destroy		= phytium_connector_destroy,
	.reset			= drm_atomic_helper_connector_reset,
	.atomic_duplicate_state	= drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state	= drm_atomic_helper_connector_destroy_state,
	.late_register		= phytium_dp_connector_register,
	.early_unregister	= phytium_dp_connector_unregister,
};

static void phytium_dp_encoder_mode_set(struct drm_encoder *encoder,
						    struct drm_display_mode *mode,
						    struct drm_display_mode *adjusted)
{
	struct phytium_dp_device *dp = encoder_to_dp_device(encoder);

	drm_mode_copy(&dp->mode, adjusted);
}

static void phytium_edp_panel_poweron(struct phytium_dp_device *phytium_dp)
{
	phytium_panel_poweron(&phytium_dp->panel);
}

static void phytium_edp_panel_poweroff(struct phytium_dp_device *phytium_dp)
{
	phytium_panel_poweroff(&phytium_dp->panel);
}

static void phytium_edp_backlight_on(struct phytium_dp_device *phytium_dp)
{
	phytium_panel_enable_backlight(&phytium_dp->panel);
}

static void phytium_edp_backlight_off(struct phytium_dp_device *phytium_dp)
{
	phytium_panel_disable_backlight(&phytium_dp->panel);
}

static void phytium_encoder_disable(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);

	if (phytium_dp->is_edp)
		phytium_edp_backlight_off(phytium_dp);

	phytium_dp_hw_disable_video(phytium_dp);

	mdelay(50);

	if (phytium_dp->is_edp)
		phytium_edp_panel_poweroff(phytium_dp);
}

void phytium_dp_adjust_link_train_parameter(struct phytium_dp_device *phytium_dp)
{
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	unsigned long link_bw, date_rate = 0, bs_limit, bs_request;
	int rate = 0;

	bs_request = phytium_dp->mode.crtc_htotal/(phytium_dp->mode.crtc_clock/1000);
	date_rate = (phytium_dp->mode.crtc_clock * display_info->bpc * 3)/8;

	for (;;) {
		bs_limit = 8192 / (phytium_dp->link_rate/1000);
		link_bw = phytium_dp->link_rate * phytium_dp->link_lane_count;
		rate = 10 * date_rate / link_bw;
		DRM_DEBUG_KMS("adjust link rate(%d), lane count(%d)\n",
			       phytium_dp->link_rate, phytium_dp->link_lane_count);
		DRM_DEBUG_KMS("for crtc_clock(%d) bs_request(%ld) bs_limit(%ld) rate(%d)\n",
			       phytium_dp->mode.crtc_clock, bs_request, bs_limit, rate);
		if ((link_dynamic_adjust && (bs_request < bs_limit) && rate < 10) ||
		   ((!link_dynamic_adjust) && (rate < 10)))
			break;
		phytium_dp_get_link_train_fallback_values(phytium_dp);
	}

	DRM_DEBUG_KMS("Try link training at Link Rate = %d, Lane count = %d\n",
		       phytium_dp->link_rate, phytium_dp->link_lane_count);
}

static void phytium_encoder_enable(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);
	int ret = 0;

	phytium_dp_hw_disable_video(phytium_dp);

	if (phytium_dp->is_edp) {
		phytium_edp_panel_poweron(phytium_dp);
		if (phytium_dp->fast_train_support)
			phytium_dp_fast_link_train(phytium_dp);
		else
			ret = phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
		phytium_dp_fast_link_train_detect(phytium_dp);
	} else {
		phytium_dp_adjust_link_train_parameter(phytium_dp);
		ret = phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
	}

	phytium_dp_hw_config_video(phytium_dp);
	if (ret == 0) {
		phytium_dp_hw_enable_video(phytium_dp);
		if (phytium_dp->has_audio)
			phytium_dp_hw_enable_audio(phytium_dp);
	}

	if (phytium_dp->is_edp) {
		phytium_edp_backlight_on(phytium_dp);
	}
}

enum drm_mode_status
phytium_encoder_mode_valid(struct drm_encoder *encoder, const struct drm_display_mode *mode)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	unsigned int requested, actual;

	switch (display_info->bpc) {
	case 10:
	case 6:
	case 8:
		break;
	default:
		DRM_INFO("not support bpc(%d)\n", display_info->bpc);
		display_info->bpc = 8;
		break;
	}

	if ((display_info->color_formats & DRM_COLOR_FORMAT_RGB444) == 0) {
		DRM_INFO("not support color_format(%d)\n", display_info->color_formats);
		display_info->color_formats = DRM_COLOR_FORMAT_RGB444;
	}

	requested = mode->clock * display_info->bpc * 3 / 1000;
	actual = phytium_dp->max_link_rate * phytium_dp->max_link_lane_count / 100;
	actual = actual * 8 / 10;
	if (requested >= actual) {
		DRM_DEBUG_KMS("requested=%d, actual=%d, clock=%d\n", requested, actual,
				mode->clock);
		return MODE_CLOCK_HIGH;
	}

	if (dc_fake_mode_enable &&
		(phytium_dp->native_mode.clock == mode->clock) &&
		(phytium_dp->native_mode.htotal == mode->htotal) &&
		(phytium_dp->native_mode.vtotal == mode->vtotal))
		return MODE_OK;

	if ((mode->hdisplay == 1600) && (mode->vdisplay == 900))
		return MODE_BAD_HVALUE;

	if ((mode->hdisplay == 1024) && (mode->clock > 78000))
		return MODE_BAD_HVALUE;

	if ((mode->hdisplay < 640) || (mode->vdisplay < 480))
		return MODE_BAD_HVALUE;

	return MODE_OK;
}

static const struct drm_encoder_helper_funcs phytium_encoder_helper_funcs = {
	.mode_set = phytium_dp_encoder_mode_set,
	.disable = phytium_encoder_disable,
	.enable  = phytium_encoder_enable,
	.mode_valid = phytium_encoder_mode_valid,
};

void phytium_dp_encoder_destroy(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);

	phytium_dp_audio_codec_fini(phytium_dp);
	drm_encoder_cleanup(encoder);
}

static const struct drm_encoder_funcs phytium_encoder_funcs = {
	.destroy = phytium_dp_encoder_destroy,
};

static const struct dp_audio_n_m phytium_dp_audio_n_m[] = {
	{ 32000,  162000, 1024, 10125 },
	{ 44100,  162000, 784,  5625  },
	{ 48000,  162000, 512,  3375  },
	{ 64000,  162000, 2048, 10125 },
	{ 88200,  162000, 1568, 5625  },
	{ 96000,  162000, 1024, 3375  },
	{ 128000, 162000, 4096, 10125 },
	{ 176400, 162000, 3136, 5625  },
	{ 192000, 162000, 2048, 3375  },
	{ 32000,  270000, 1024, 16875 },
	{ 44100,  270000, 784,  9375  },
	{ 48000,  270000, 512,  5625  },
	{ 64000,  270000, 2048, 16875 },
	{ 88200,  270000, 1568, 9375  },
	{ 96000,  270000, 1024, 5625  },
	{ 128000, 270000, 4096, 16875 },
	{ 176400, 270000, 3136, 9375  },
	{ 192000, 270000, 2048, 5625  },
	{ 32000,  540000, 1024, 33750 },
	{ 44100,  540000, 784,  18750 },
	{ 48000,  540000, 512,  11250 },
	{ 64000,  540000, 2048, 33750 },
	{ 88200,  540000, 1568, 18750 },
	{ 96000,  540000, 1024, 11250 },
	{ 128000, 540000, 4096, 33750 },
	{ 176400, 540000, 3136, 18750 },
	{ 192000, 540000, 2048, 11250 },
	{ 32000,  810000, 1024, 50625 },
	{ 44100,  810000, 784,  28125 },
	{ 48000,  810000, 512,  16875 },
	{ 64000,  810000, 2048, 50625 },
	{ 88200,  810000, 1568, 28125 },
	{ 96000,  810000, 1024, 16875 },
	{ 128000, 810000, 4096, 50625 },
	{ 176400, 810000, 3136, 28125 },
	{ 192000, 810000, 2048, 16875 },
};

static int phytium_dp_audio_get_eld(struct device *dev, void *data, u8 *buf, size_t len)
{
	struct phytium_dp_device *phytium_dp = data;

	memcpy(buf, phytium_dp->connector.eld, min(sizeof(phytium_dp->connector.eld), len));

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
static int phytium_dp_audio_digital_mute(struct device *dev, void *data, bool enable)
{
	struct phytium_dp_device *phytium_dp = data;

	phytium_dp_hw_audio_digital_mute(phytium_dp, enable);

	return 0;
}
#else
static int phytium_dp_audio_mute_stream(struct device *dev, void *data, bool enable, int direction)
{
	struct phytium_dp_device *phytium_dp = data;

	phytium_dp_hw_audio_digital_mute(phytium_dp, enable);

	return 0;
}
#endif

const struct dp_audio_n_m *phytium_dp_audio_get_n_m(int link_rate, int sample_rate)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(phytium_dp_audio_n_m); i++) {
		if (sample_rate == phytium_dp_audio_n_m[i].sample_rate
			&& link_rate == phytium_dp_audio_n_m[i].link_rate)
			return &phytium_dp_audio_n_m[i];
	}

	return NULL;
}

static int phytium_dp_audio_hw_params(struct device *dev, void *data,
						 struct hdmi_codec_daifmt *daifmt,
						 struct hdmi_codec_params *params)
{
	struct phytium_dp_device *phytium_dp = data;
	int ret = 0;
	struct audio_info audio_info = {
		.sample_width = params->sample_width,
		.sample_rate = params->sample_rate,
		.channels = params->channels,
	};

	if (daifmt->fmt != HDMI_I2S) {
		DRM_ERROR("invalid audio format %d\n", daifmt->fmt);
		ret = -EINVAL;
		goto failed;
	}

	ret = phytium_dp_hw_audio_hw_params(phytium_dp, audio_info);

failed:
	return ret;
}

static void phytium_dp_audio_shutdown(struct device *dev, void *data)
{
	struct phytium_dp_device *phytium_dp = data;

	phytium_dp_hw_audio_shutdown(phytium_dp);
}

static void handle_plugged_change(struct phytium_dp_device *phytium_dp, bool plugged)
{
	if (phytium_dp->plugged_cb && phytium_dp->codec_dev)
		phytium_dp->plugged_cb(phytium_dp->codec_dev, plugged);
}

static int phytium_dp_audio_hook_plugged_cb(struct device *dev, void *data,
				       hdmi_codec_plugged_cb fn,
				       struct device *codec_dev)
{
	struct phytium_dp_device *phytium_dp = data;
	bool plugged;

	phytium_dp->plugged_cb = fn;
	phytium_dp->codec_dev = codec_dev;

	if ((phytium_dp->connector.status == connector_status_connected) && phytium_dp->has_audio)
		plugged = true;
	else
		plugged = false;

	handle_plugged_change(phytium_dp, plugged);
	return 0;
}


static const struct hdmi_codec_ops phytium_audio_codec_ops = {
	.hw_params = phytium_dp_audio_hw_params,
	.audio_shutdown = phytium_dp_audio_shutdown,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	.digital_mute = phytium_dp_audio_digital_mute,
#else
	.mute_stream = phytium_dp_audio_mute_stream,
#endif
	.get_eld = phytium_dp_audio_get_eld,
	.hook_plugged_cb = phytium_dp_audio_hook_plugged_cb,
};

static int phytium_dp_audio_codec_init(struct phytium_dp_device *phytium_dp,
				       const int port)
{
	struct device *dev = phytium_dp->dev->dev;
	struct hdmi_codec_pdata codec_data = {
		.i2s = 1,
		.spdif = 0,
		.ops = &phytium_audio_codec_ops,
		.max_i2s_channels = 2,
		.data = phytium_dp,
	};

	phytium_dp->audio_pdev = platform_device_register_data(dev, HDMI_CODEC_DRV_NAME,
							       codec_id + port,
							       &codec_data, sizeof(codec_data));

	return PTR_ERR_OR_ZERO(phytium_dp->audio_pdev);
}

static void phytium_dp_audio_codec_fini(struct phytium_dp_device *phytium_dp)
{
	if (!PTR_ERR_OR_ZERO(phytium_dp->audio_pdev))
		platform_device_unregister(phytium_dp->audio_pdev);
	phytium_dp->audio_pdev = NULL;
}

static long phytium_dp_aux_transfer(struct drm_dp_aux *aux, struct drm_dp_aux_msg *msg)
{
	struct phytium_dp_device *phytium_dp = container_of(aux, struct phytium_dp_device, aux);
	long ret = 0;

	DRM_DEBUG_KMS("msg->size: 0x%lx\n", msg->size);

	if (WARN_ON(msg->size > 16))
		return -E2BIG;

	switch (msg->request & ~DP_AUX_I2C_MOT) {
	case DP_AUX_NATIVE_WRITE:
	case DP_AUX_I2C_WRITE:
	case DP_AUX_I2C_WRITE_STATUS_UPDATE:
		ret = phytium_dp_hw_aux_transfer_write(phytium_dp, msg);
		DRM_DEBUG_KMS("aux write reply:0x%x ret:0x%lx\n", msg->reply, ret);
		break;
	case DP_AUX_NATIVE_READ:
	case DP_AUX_I2C_READ:
		ret = phytium_dp_hw_aux_transfer_read(phytium_dp, msg);
		DRM_DEBUG_KMS("aux read ret:0x%lx\n", ret);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static void phytium_dp_aux_init(struct phytium_dp_device *phytium_dp)
{
	drm_dp_aux_init(&phytium_dp->aux);
	phytium_dp->aux.name = kasprintf(GFP_KERNEL, "dp-%d", phytium_dp->port);
	phytium_dp->aux.transfer = phytium_dp_aux_transfer;
}

int phytium_get_encoder_crtc_mask(struct phytium_dp_device *phytium_dp, int port)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int i, mask = 0;

	for_each_pipe_masked(priv, i) {
		if (i != port)
			mask++;
		else
			break;
	}

	return BIT(mask);
}

static bool phytium_dp_is_edp(struct phytium_dp_device *phytium_dp, int port)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;

	if (priv->info.edp_mask & BIT(port))
		return true;
	else
		return false;
}

static bool phytium_edp_init_connector(struct phytium_dp_device *phytium_dp)
{
	enum drm_connector_status status;
	struct drm_connector *connector = &phytium_dp->connector;

	phytium_edp_panel_poweron(phytium_dp);

	status = phytium_dp_detect_dpcd(phytium_dp);
	if (status == connector_status_disconnected) {
		DRM_ERROR("detect edp dpcd failed\n");
		return false;
	}

	phytium_dp->edp_edid = drm_get_edid(connector, &phytium_dp->aux.ddc);
	if (!phytium_dp->edp_edid) {
		DRM_ERROR("get edp edid failed\n");
		return false;
	}

	connector->status = status;
	phytium_dp->max_link_rate = phytium_dp->common_rates[phytium_dp->num_common_rates-1];
	phytium_dp->max_link_lane_count = phytium_dp->common_max_lane_count;
	phytium_dp->link_rate = phytium_dp->max_link_rate;
	phytium_dp->link_lane_count = phytium_dp->max_link_lane_count;
	DRM_DEBUG_KMS("common_max_lane_count: %d, common_max_rate:%d\n",
		       phytium_dp->max_link_lane_count, phytium_dp->max_link_rate);

	return true;
}

static void phytium_edp_fini_connector(struct phytium_dp_device *phytium_dp)
{
	if (phytium_dp->edp_edid)
		kfree(phytium_dp->edp_edid);

	phytium_dp->edp_edid = NULL;
	phytium_edp_panel_poweroff(phytium_dp);

	return;
}

int phytium_dp_resume(struct drm_device *drm_dev)
{
	struct phytium_dp_device *phytium_dp;
	struct drm_encoder *encoder;
	int ret = 0;

	drm_for_each_encoder(encoder, drm_dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		if (phytium_dp->is_edp) {
			phytium_edp_backlight_off(phytium_dp);
			phytium_edp_panel_poweroff(phytium_dp);
		}
		ret = phytium_dp_hw_init(phytium_dp);
		if (ret) {
			DRM_ERROR("failed to initialize dp %d\n", phytium_dp->port);
			return -EIO;
		}
	}

	return 0;
}

int phytium_dp_init(struct drm_device *dev, int port)
{
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_dp_device *phytium_dp = NULL;
	int ret, type;

	DRM_DEBUG_KMS("%s: port %d\n", __func__, port);
	phytium_dp = kzalloc(sizeof(*phytium_dp), GFP_KERNEL);
	if (!phytium_dp) {
		ret = -ENOMEM;
		goto failed_malloc_dp;
	}

	phytium_dp->dev = dev;
	phytium_dp->port = port;

	if (IS_PX210(priv)) {
		px210_dp_func_register(phytium_dp);
		priv->dp_reg_base[port] = PX210_DP_BASE(port);
		priv->phy_access_base[port] = PX210_PHY_ACCESS_BASE(port);
	} else if (IS_PE220X(priv)) {
		pe220x_dp_func_register(phytium_dp);
		priv->dp_reg_base[port] = PE220X_DP_BASE(port);
		priv->phy_access_base[port] = PE220X_PHY_ACCESS_BASE(port);
	}

	if (phytium_dp_is_edp(phytium_dp, port)) {
		phytium_dp->is_edp = true;
		type = DRM_MODE_CONNECTOR_eDP;
		phytium_dp_panel_init_backlight_funcs(phytium_dp);
		phytium_edp_backlight_off(phytium_dp);
		phytium_edp_panel_poweroff(phytium_dp);
	} else {
		phytium_dp->is_edp = false;
		type = DRM_MODE_CONNECTOR_DisplayPort;
	}

	ret = phytium_dp_hw_init(phytium_dp);
	if (ret) {
		DRM_ERROR("failed to initialize dp %d\n", phytium_dp->port);
		goto failed_init_dp;
	}

	ret = drm_encoder_init(dev, &phytium_dp->encoder,
			       &phytium_encoder_funcs,
			       DRM_MODE_ENCODER_TMDS, "DP %d", port);
	if (ret) {
		DRM_ERROR("failed to initialize encoder with drm\n");
		goto failed_encoder_init;
	}
	drm_encoder_helper_add(&phytium_dp->encoder, &phytium_encoder_helper_funcs);
	phytium_dp->encoder.possible_crtcs = phytium_get_encoder_crtc_mask(phytium_dp, port);

	phytium_dp->connector.dpms   = DRM_MODE_DPMS_OFF;
	phytium_dp->connector.polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;
	ret = drm_connector_init(dev, &phytium_dp->connector, &phytium_connector_funcs,
				 type);
	if (ret) {
		DRM_ERROR("failed to initialize connector with drm\n");
		goto failed_connector_init;
	}
	drm_connector_helper_add(&phytium_dp->connector, &phytium_connector_helper_funcs);
	drm_connector_attach_encoder(&phytium_dp->connector, &phytium_dp->encoder);

	ret = phytium_dp_audio_codec_init(phytium_dp, port);
	if (ret) {
		DRM_ERROR("failed to initialize audio codec\n");
		goto failed_connector_init;
	}

	phytium_dp->train_retry_count = 0;
	INIT_WORK(&phytium_dp->train_retry_work, phytium_dp_train_retry_work_fn);
	drm_connector_register(&phytium_dp->connector);

	return 0;
failed_connector_init:
failed_encoder_init:
failed_init_dp:
	kfree(phytium_dp);
failed_malloc_dp:
	return ret;
}
