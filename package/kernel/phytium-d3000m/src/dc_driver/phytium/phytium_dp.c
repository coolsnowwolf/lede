// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
#include <linux/acpi.h>
#endif

#include "phytium_dp.h"
#include "phytium_dp_reg.h"
#include "phytium_se_communicate.h"
#include "ftd330_dp.h"
#include "phytium_panel.h"
#include "phytium_edp_pwm.h"
#include "phytium_psr.h"
#include "phytium_vrr.h"
#include "phytium_dp_debugfs.h"
#include "uapi/linux/media-bus-format.h"
#include "ftd330_crtc.h"
#include "FTD330/ftd330_dc.h"

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
static int phytium_rate[] = {162000, 270000, 540000, 810000};
#endif

// #ifdef DRM_DEBUG_KMS
// #undef DRM_DEBUG_KMS
// #endif

// #define DRM_DEBUG_KMS pr_info

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
#define SINK_AVALIABLE 1
#else
#define SINK_AVALIABLE 0
#endif


static int codec_id = PHYTIUM_DP_AUDIO_ID;
static void handle_plugged_change(struct phytium_dp_device *phytium_dp, bool plugged);
static void phytium_edp_init_connector(struct phytium_dp_device *phytium_dp);
static void phytium_edp_fini_connector(struct phytium_dp_device *phytium_dp);
#ifdef CONFIG_PHYTIUM_EDP_BL
static void phytium_edp_panel_poweron(struct phytium_dp_device *phytium_dp);
static void phytium_edp_panel_poweroff(struct phytium_dp_device *phytium_dp);
static void phytium_edp_backlight_off(struct phytium_dp_device *phytium_dp);
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
static void phytium_low_fps_dplp_init(struct phytium_dp_device *phytium_dp);
#endif
#ifdef CONFIG_PHYTIUM_PSR
static void phytium_psr_dplp_init(struct phytium_dp_device *phytium_dp);
#endif
#if defined(CONFIG_PHYTIUM_LOW_FPS) || defined(CONFIG_PHYTIUM_PSR)
static void phytium_psr_lowfps_dplp_deinit(struct phytium_dp_device *phytium_dp);
#endif

static struct drm_display_mode tcl_custom_modes[] = {
	/*2880x1800-90hz*/
	{ DRM_MODE("2880x1800", DRM_MODE_TYPE_DRIVER, 522576, 2880, 2928,
			2960, 3040, 0, 1800, 1803, 1809, 1910, 0,
			DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC) },
};
static struct drm_display_mode tm_custom_modes[] = {
	/*2880x1800-90hz*/
	{ DRM_MODE("2880x1800", DRM_MODE_TYPE_DRIVER, 516830, 2880, 2928,
			2960, 3040, 0, 1800, 1803, 1809, 1889, 0,
			DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_NVSYNC) },
};


static int phytium_port_virtual_to_physical(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int i = 0;
	int count = 0;
	bool found = false;

	for (i = 0; i < DISPLAY_NUM; i++) {
		if (priv->info.pipe_mask & BIT(i)) {
			if (count == port) {
				found = true;
				break;
			} else {
				count++;
			}
		}
	}

	if (found) {
		return i;
	} else {
		pr_err("%s: no right dp port found\n", __func__);
		return -1;
	}
}


void phytium_phy_writel(struct phytium_dp_device *phytium_dp, uint32_t address, uint32_t data)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
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
	struct ftd330_drm_private *priv = dev->dev_private;
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

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
static int
phytium_dp_hw_aux_transfer_write(struct phytium_dp_device *phytium_dp, struct drm_dp_aux_msg *msg)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	unsigned int i = 0, j = 0;
	unsigned int cmd = 0;
	unsigned int aux_status = 0, interrupt_status = 0;
	unsigned char *data = msg->buffer;
	int count_timeout = 0;
	long ret = 0;

	for (i = 0; i < 3; i++) {
		/* clear X100_DP_INTERRUPT_RAW_STATUS */
		phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
		phytium_writel_reg(priv, msg->address, group_offset, PHYTIUM_DP_AUX_ADDRESS);
		for (j = 0; j < msg->size; j++) {
			phytium_writeb_reg(priv, data[j], group_offset, PHYTIUM_DP_AUX_WRITE_FIFO);
			#if DEBUG_LOG
				pr_info("FTD330 aux data write is data[%d] %d\n", j, data[j]);
			#endif
		}

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
				//DRM_DEBUG_KMS("aux wait exit\n");
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
			//DRM_DEBUG_KMS("aux write reply received succussful\n");
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
	struct ftd330_drm_private *priv = dev->dev_private;
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
				//DRM_DEBUG_KMS("aux wait exit\n");
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
			//DRM_DEBUG_KMS("aux read reply received succussful\n");
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

	for (i = 0; i < ret; i++) {
		data[i] = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_AUX_REPLY_DATA);
	}

out:
	return ret;
}

static long phytium_dp_aux_transfer(struct drm_dp_aux *aux, struct drm_dp_aux_msg *msg)
{
	struct phytium_dp_device *phytium_dp = container_of(aux, struct phytium_dp_device, aux);
	long ret = 0;

	//DRM_DEBUG_KMS("msg->size: 0x%lx\n", msg->size);

	if (WARN_ON(msg->size > 16))
		return -E2BIG;

	switch (msg->request & ~DP_AUX_I2C_MOT) {
	case DP_AUX_NATIVE_WRITE:
	case DP_AUX_I2C_WRITE:
	case DP_AUX_I2C_WRITE_STATUS_UPDATE:
			ret = phytium_dp_hw_aux_transfer_write(phytium_dp, msg);
			//DRM_DEBUG_KMS("aux write reply:0x%x ret:0x%lx\n", msg->reply, ret);
			break;
	case DP_AUX_NATIVE_READ:
	case DP_AUX_I2C_READ:
			ret = phytium_dp_hw_aux_transfer_read(phytium_dp, msg);
			//DRM_DEBUG_KMS("aux read ret:0x%lx\n", ret);
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
#endif


bool is_dp_powered(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t group_offset = priv->dp_reg_base[phytium_dp->port];
	uint32_t dp_ver = 0;

	dp_ver = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_DISPLAYPORT_VERSION);

	if (dp_ver) {
		return true;	
	} else {
		return false;
	}
}

static void phytium_dp_clean_display_modes(struct phytium_dp_device *phytium_dp) {
	struct phytium_display_mode *mode, *tmp;

	list_for_each_entry_safe(mode, tmp, &phytium_dp->phytium_mode.list, list) {
		list_del(&mode->list);
		kfree(mode);
	}
}

static void phytium_display_mode_copy(struct phytium_display_mode *phytium_mode,
								const struct drm_display_mode *mode)
{
	phytium_mode->clock = mode->clock;
	phytium_mode->h_active = mode->hdisplay;
	phytium_mode->h_total = mode->htotal;
	phytium_mode->h_sync_start = mode->hsync_start;
	phytium_mode->h_sync_end = mode->hsync_end;
	phytium_mode->v_active = mode->vdisplay;
	phytium_mode->v_total = mode->vtotal;
	phytium_mode->v_sync_start = mode->vsync_start;
	phytium_mode->v_sync_end = mode->vsync_end;
	phytium_mode->flags = mode->flags;
	phytium_mode->type = mode->type;
	phytium_mode->freq = drm_mode_vrefresh(mode);
}

static bool phytium_display_mode_compare(struct phytium_display_mode *phytium_mode,
								const struct drm_display_mode *mode)
{
	if (!phytium_mode || !mode)
		return false;

	if (phytium_mode->clock == mode->clock &&
		phytium_mode->h_active == mode->hdisplay &&
		phytium_mode->h_total == mode->htotal &&
		phytium_mode->h_sync_start == mode->hsync_start &&
		phytium_mode->h_sync_end == mode->hsync_end &&
		phytium_mode->v_active == mode->vdisplay &&
		phytium_mode->v_total == mode->vtotal &&
		phytium_mode->v_sync_start == mode->vsync_start &&
		phytium_mode->v_sync_end == mode->vsync_end &&
		phytium_mode->flags == mode->flags &&
		phytium_mode->type == mode->type)
		return true;
	else
		return false;
}

static bool phytium_drm_mode_in_phytium_mode(struct phytium_dp_device *phytium_dp,
								const struct drm_display_mode *mode)
{
	struct phytium_display_mode *phytium_mode = NULL;

	if (!list_empty(&phytium_dp->phytium_mode.list)) {
		list_for_each_entry(phytium_mode, &phytium_dp->phytium_mode.list, list) {
			if (phytium_display_mode_compare(phytium_mode, mode))
				return true;
			else
				return false;
		}
	}
	return false;
}

static void phytium_get_native_mode(struct phytium_dp_device *phytium_dp)
{
	struct drm_display_mode *t, *mode;
	struct drm_connector *connector = &phytium_dp->connector;
	struct drm_display_mode *native_mode = &phytium_dp->native_mode;
	struct drm_display_mode temp_mode;

	list_for_each_entry_safe(mode, t, &connector->probed_modes, head) {
		if (mode->type & DRM_MODE_TYPE_PREFERRED) {
			memcpy(&temp_mode, mode, sizeof(*mode));
		}
	}

	list_for_each_entry_safe(mode, t, &connector->probed_modes, head) {
			if(mode->hdisplay == temp_mode.hdisplay &&
				mode->vdisplay == temp_mode.vdisplay &&
				drm_mode_vrefresh(mode) <= 61 &&
				drm_mode_vrefresh(mode) >= 59) {
				if (mode->hdisplay != native_mode->hdisplay ||
					mode->vdisplay != native_mode->vdisplay) {
					memcpy(native_mode, mode, sizeof(*mode));
					drm_mode_set_crtcinfo(native_mode, 0);
					break;
			}
		}
	}
	FTD330_LOG("native_mode is (%dx%d),clock is %d\n",
			native_mode->hdisplay, native_mode->vdisplay, native_mode->clock);
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

		// if (2 * common_mode[i].w < native_mode->hdisplay ||
		// 	2 * common_mode[i].h < native_mode->vdisplay)
		// 	continue;

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
		strscpy(mode->name, common_mode[i].name, DRM_DISPLAY_MODE_LEN);
		drm_mode_probed_add(&phytium_dp->connector, mode);
		ret++;
	}

	return ret;
}

static u32 edid_extract_panel_id(struct edid *edid)
{
	return (u32)edid->mfg_id[0] << 24	|
			(u32)edid->mfg_id[1] << 16	|
			(u32)EDID_PRODUCT_ID(edid);
}
static int phytium_do_add_custom_modes(struct drm_connector *connector,
									struct drm_display_mode *custom_modes,
									int count)
{
	int i = 0;
	int num_mode = 0;
	struct drm_display_mode *ptr = NULL;
	struct drm_display_mode *mode = NULL;
	struct drm_device *dev = connector->dev;

	for (i = 0; i < count; i++) {
		ptr = &custom_modes[i];

		mode = drm_mode_duplicate(dev, ptr);
		if (mode) {
			drm_mode_probed_add(connector, mode);
			num_mode++;
			DRM_DEBUG_KMS("Add custom mode %dx%d-%dHz (clk:%d)\n",
					mode->hdisplay, mode->vdisplay,
					drm_mode_vrefresh(mode), mode->clock);
			}
		}
	return num_mode;
}

static int phytium_add_custom_modes(struct drm_connector *connector, struct edid *edid)
{
	int num_mode = 0;
	uint32_t panel_id = edid_extract_panel_id(edid);

	switch (panel_id) {
	/* Workaround for some monitors which needs add custom modes */
	case phytium_edid_encode_panel_id('C', 'S', 'W', 0x1464):
		num_mode += phytium_do_add_custom_modes(connector,
						tcl_custom_modes, ARRAY_SIZE(tcl_custom_modes));
		break;
	case phytium_edid_encode_panel_id('T', 'M', 'A', 0x2056):
		num_mode += phytium_do_add_custom_modes(connector,
						tm_custom_modes, ARRAY_SIZE(tm_custom_modes));
		break;
	default:
		break;
	}
	return num_mode;

}

static int phytium_connector_get_modes(struct drm_connector *connector)
{
    struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
    struct edid *edid;
    int ret = 0;

	FTD330_LOG_TRACE;

	if (!list_empty(&phytium_dp->phytium_mode.list))
		phytium_dp_clean_display_modes(phytium_dp);

	if (phytium_dp->is_edp)
		edid = phytium_dp->edp_edid;
	else
   		edid = drm_get_edid(connector, &phytium_dp->aux.ddc);

    if (edid && drm_edid_is_valid(edid)) {
        drm_connector_update_edid_property(connector, edid);
        ret = drm_add_edid_modes(connector, edid);
		ret += phytium_add_custom_modes(connector, edid);
        phytium_dp->has_audio = drm_detect_monitor_audio(edid);
        phytium_get_native_mode(phytium_dp);
        if (dc_fake_mode_enable && phytium_dp->is_edp)
                ret += phytium_connector_add_common_modes(phytium_dp);
    } else {
        drm_connector_update_edid_property(connector, NULL);
        phytium_dp->has_audio = false;
    }

	if (!phytium_dp->is_edp && edid)
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

static void
phytium_dp_set_bios_common_rates_lanes(struct phytium_dp_device *phytium_dp, 
										struct edid *edid)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct bios_table_info *bios_info = &priv->info.bios_info;
	int i = 0;

	if (edid && drm_edid_is_valid(edid)) {
		uint32_t panel_id = edid_extract_panel_id(edid);
		for (i = 0; i < bios_info->panel_count; i++){
			DRM_DEBUG_KMS("%s panel_id is 0x%x,"
							"bios_info->panels[%d].panel_id is 0x%x.\n",
							__func__, panel_id, i,
							bios_info->panels[i].panel_id);
			if (bios_info->panels[i].valid &&
				panel_id == bios_info->panels[i].panel_id) {
				phytium_dp->num_common_rates =
					get_common_rates(phytium_dp->common_rates,
					phytium_dp->num_common_rates,
					phytium_dp->sink_rates,
					bios_info->panels[i].num_link_rate,
					phytium_dp->common_rates);
				if (WARN_ON(phytium_dp->num_common_rates == 0)) {
				phytium_dp->common_rates[0] = 162000;
				phytium_dp->num_common_rates = 1;
				}

				phytium_dp->common_max_lane_count =
					min(phytium_dp->common_max_lane_count,
						(int)bios_info->panels[i].max_lane_count);
				DRM_DEBUG_KMS("After parsing ACPI, the max common rate is %d"
							"and the common lane count is %d.\n",
							phytium_dp->common_rates[phytium_dp->num_common_rates-1],
							phytium_dp->common_max_lane_count);
				break;
			}
		}
	}
}

static int phytium_dp_read_extended_dpcd_caps(struct drm_dp_aux *aux,
	u8 dpcd[DP_RECEIVER_CAP_SIZE])
{
	u8 dpcd_ext[DP_RECEIVER_CAP_SIZE];
	int ret;

	/*
	* Prior to DP1.3 the bit represented by
	* DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT was reserved.
	* If it is set DP_DPCD_REV at 0000h could be at a value less than
	* the true capability of the panel. The only way to check is to
	* then compare 0000h and 2200h.
	*/
	if (!(dpcd[DP_TRAINING_AUX_RD_INTERVAL] &
		DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT))
		return 0;

	ret = drm_dp_dpcd_read(aux, DP_DP13_DPCD_REV, &dpcd_ext,
							sizeof(dpcd_ext));
	if (ret < 0)
		return ret;
	if (ret != sizeof(dpcd_ext))
		return -EIO;

	if (dpcd[DP_DPCD_REV] > dpcd_ext[DP_DPCD_REV]) {
		DRM_ERROR("Extended DPCD rev less than base DPCD rev (%d > %d)\n",
	     dpcd[DP_DPCD_REV], dpcd_ext[DP_DPCD_REV]);
		return 0;
	}

	if (!memcmp(dpcd, dpcd_ext, sizeof(dpcd_ext)))
		return 0;

	DRM_DEBUG_KMS("Base DPCD: %*ph\n", DP_RECEIVER_CAP_SIZE, dpcd);

	memcpy(dpcd, dpcd_ext, sizeof(dpcd_ext));

	return 0;
}

static bool phytium_dp_get_dpcd(struct phytium_dp_device *phytium_dp)
{
	int ret;
	unsigned char sink_count = 0;

	/* get dpcd capability,but don't check data error; so check revision */
	ret = drm_dp_dpcd_read(&phytium_dp->aux, 0x00, phytium_dp->dpcd,
			       sizeof(phytium_dp->dpcd));
	if (ret < 0) {
		DRM_ERROR("port %d get DPCD capability base DPCD capability fail\n", phytium_dp->port);
		return false;
	}

	ret =  phytium_dp_read_extended_dpcd_caps(&phytium_dp->aux, phytium_dp->dpcd);
	DRM_DEBUG_KMS("dp-%d,get extend max link rate is 0x%x \n",
					phytium_dp->port, phytium_dp->dpcd[DP_MAX_LINK_RATE]);

	if (ret < 0) {
		DRM_ERROR("port %d get extend DPCD capability fail\n", phytium_dp->port);
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

static bool phytium_dp_coding_8b10b_need_enable(unsigned char test_pattern)
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

static bool phytium_dp_scrambled_need_enable(unsigned char test_pattern)
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
	struct ftd330_drm_private *priv = dev->dev_private;
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
	struct ftd330_drm_private *priv = dev->dev_private;
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
	struct ftd330_drm_private *priv = dev->dev_private;
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

static void phytium_dp_registers_init(struct ftd330_drm_private *priv, uint32_t port)
{
	uint32_t group_offset = priv->dp_reg_base[port];

	FTD330_LOG_TRACE;
	phytium_writel_reg(priv, 1, group_offset, PHYTIUM_DP_HPD_STATE_RESET);
	phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DP_HPD_STATE_RESET);
}

static bool phytium_dp_hw_audio_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port, config;
	uint32_t group_offset = priv->dp_reg_base[port];

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
	return config ? true : false;
}

void phytium_dp_hw_enable_audio(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int config = 0, config1, data_window = 0;
	const struct dp_audio_n_m *n_m = NULL;
	uint32_t group_offset = priv->dp_reg_base[port];
	int h_porch_percent = 0;
	int data_window_factor = 0;

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);
	phytium_writel_reg(priv, CHANNEL_MUTE_ENABLE, group_offset, PHYTIUM_DP_SEC_AUDIO_ENABLE);


       if ((phytium_dp->mode.hdisplay == 3840) &&
                        (phytium_dp->mode.vdisplay == 2160) &&
                        (drm_mode_vrefresh(&phytium_dp->mode) > 59)) {
                h_porch_percent = (phytium_dp->mode.htotal - phytium_dp->mode.hdisplay) * 100 / phytium_dp->mode.htotal;
                if (h_porch_percent <= 4) {
                        data_window_factor = 50;
                } else {
                        data_window_factor = 90;
                }
        } else {
                data_window_factor = 90;
        }

	data_window = data_window_factor*(phytium_dp->link_rate)/100
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
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SECONDARY_STREAM_DISABLE,
			   group_offset, PHYTIUM_DP_SECONDARY_STREAM_ENABLE);
}

static void phytium_dp_hw_audio_digital_mute(struct phytium_dp_device *phytium_dp, bool enable)
{
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
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
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0, data_window = 0;
	const struct dp_audio_n_m *n_m = NULL;
	uint32_t fs, ws, fs_accurac;
	uint32_t group_offset = priv->dp_reg_base[port];
	int data_window_factor = 0;
	int h_porch_percent = 0;

	DRM_DEBUG_KMS("%s:set port%d sample_rate(%d) channels(%d) sample_width(%d)\n",
			__func__, phytium_dp->port, audio_info.sample_rate,
			audio_info.channels, audio_info.sample_width);

	phytium_writel_reg(priv, INPUT_SELECT_I2S, group_offset, PHYTIUM_DP_SEC_INPUT_SELECT);
	phytium_writel_reg(priv, APB_CLOCK_100/audio_info.sample_rate,
			   group_offset, PHYTIUM_DP_SEC_DIRECT_CLKDIV);
	phytium_writel_reg(priv, audio_info.channels & CHANNEL_MASK,
			   group_offset, PHYTIUM_DP_SEC_CHANNEL_COUNT);
	phytium_writel_reg(priv, CHANNEL_MAP_DEFAULT, group_offset, PHYTIUM_DP_SEC_CHANNEL_MAP);
	
      if ((phytium_dp->mode.hdisplay == 3840) &&
                        (phytium_dp->mode.vdisplay == 2160) &&
                        (drm_mode_vrefresh(&phytium_dp->mode) > 59)) {
                h_porch_percent = (phytium_dp->mode.htotal - phytium_dp->mode.hdisplay) * 100 / phytium_dp->mode.htotal;
                if (h_porch_percent <= 4) {
                        data_window_factor = 50;
                } else {
                        data_window_factor = 90;
                }
        } else {
                data_window_factor = 90;
        }

	data_window = data_window_factor*(phytium_dp->link_rate)/100
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

static void phytium_dp_hw_disable_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,
			   group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
}

static bool phytium_dp_hw_video_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port, config;
	uint32_t group_offset = priv->dp_reg_base[port];

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
	return config ? true : false;
}

static void phytium_dp_hw_enable_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, SST_MST_SOURCE_0_ENABLE,
			   group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
}

static void phytium_dp_hw_config_video(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	unsigned long link_bw, date_rate = 0;
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	struct phytium_display_mode *phytium_mode = NULL;
	unsigned char tu_size = 64;
	unsigned long data_per_tu = 0;
	int symbols_per_tu, frac_symbols_per_tu, symbol_count, udc, value;

	phytium_dp->bpc = display_info->bpc;
	list_for_each_entry(phytium_mode, &phytium_dp->phytium_mode.list, list) {
	if (phytium_display_mode_compare(phytium_mode, &phytium_dp->mode) &&
		phytium_mode->reduced_bpc) {
			switch (phytium_mode->bpc) {
			case 10:
				phytium_dp->bpc = 10;
				break;
			case 6:
				phytium_dp->bpc = 6;
				break;
			default:
				phytium_dp->bpc = 8;
				break;
			}
		}
	}

	/* cal M/N and tu_size */
	phytium_writel_reg(priv, phytium_dp->mode.crtc_clock/10, group_offset, PHYTIUM_DP_M_VID);
	phytium_writel_reg(priv, phytium_dp->link_rate/10, group_offset, PHYTIUM_DP_N_VID);
	link_bw = phytium_dp->link_rate * phytium_dp->link_lane_count;
	date_rate = (phytium_dp->mode.crtc_clock * phytium_dp->bpc * 3)/8;

	/* mul 10 for register setting */
	data_per_tu = 10*tu_size * date_rate/link_bw;
	symbols_per_tu = (data_per_tu/10)&0xff;
	if (symbols_per_tu == 63)
		frac_symbols_per_tu = 0;
	else
		frac_symbols_per_tu = (data_per_tu%10*16/10) & 0xf;
	phytium_writel_reg(priv, frac_symbols_per_tu<<24 | symbols_per_tu<<16 | tu_size,
			   group_offset, PHYTIUM_DP_TRANSFER_UNIT_SIZE);

	symbol_count = (phytium_dp->mode.crtc_hdisplay*phytium_dp->bpc*3 + 7)/8;
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
		value = value & (~PHYYNC_POLARITY_LOW);
	else
		value = value | PHYYNC_POLARITY_LOW;
	phytium_writel_reg(priv, value, group_offset, PHYTIUM_DP_MAIN_LINK_POLARITY);

	switch (phytium_dp->bpc) {
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
		value = value | USER_PHYYNC_POLARITY_HIGH;
	else
		value = value & (~USER_PHYYNC_POLARITY_HIGH);
	phytium_writel_reg(priv, value, group_offset, PHYTIUM_DP_USER_SYNC_POLARITY);
	phytium_dp->freq = drm_mode_vrefresh(&phytium_dp->mode);

	phytium_writel_reg(priv, DISABLE, group_offset, PHYTIUM_DP_IDLE_PATTERN_DISBALE);
}

static void phytium_dp_hw_disable_output(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_DISABLE,
			   group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
}

static void phytium_dp_hw_enable_output(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, LINK_SOFT_RESET, group_offset, PHYTIUM_DP_SOFT_RESET);
	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_ENABLE,
			   group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
}

static void phytium_dp_hw_enable_input_source(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, VIRTUAL_SOURCE_0_ENABLE,
			   group_offset, PHYTIUM_INPUT_SOURCE_ENABLE);
}

static void phytium_dp_hw_disable_input_source(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;

	phytium_writel_reg(priv, (~VIRTUAL_SOURCE_0_ENABLE)&VIRTUAL_SOURCE_0_ENABLE_MASK,
			   priv->dp_reg_base[port], PHYTIUM_INPUT_SOURCE_ENABLE);
}

static bool phytium_dp_hw_output_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	int config = 0;

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	return config ? true : false;
}


#ifdef CONFIG_PHYTIUM_PSR
static int phytium_dplp_hw_get_psr_state(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t val = 0;
	uint32_t group_offset = priv->dplp_reg_base[port];

	val = phytium_dplp_read_reg(priv, group_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_STATE);

	if (val & PSR_AVAILABLE) {

		FTD330_LOG("get psr avaliable irq\n");
		phytium_dp->dplp_frame_compare_state.psr_available = true;
		phytium_dp->dplp_frame_compare_state.frame_change_in_psr = false;
#ifdef CONFIG_PHYTIUM_PSR_SF_UPDATE
		phytium_dp->dplp_frame_compare_state.psr_exit = false;
#endif
		phytium_dplp_write_reg(priv, PSR_AVAILABLE, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		return 1;
	} else if (val & FRAME_CHANGE_IN_PSR) {
		FTD330_LOG("get FRAME_CHANGE_IN_PSR irq\n");
		phytium_dp->dplp_frame_compare_state.frame_change_in_psr = true;
		phytium_dp->dplp_frame_compare_state.psr_available = false;
#ifdef CONFIG_PHYTIUM_PSR_SF_UPDATE
		phytium_dp->psr.frame_change_irq_nums += 1;
#endif
		phytium_dplp_write_reg(priv, FRAME_CHANGE_IN_PSR, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		return 1;
	}
	return 0;
}
#endif

#ifdef CONFIG_PHYTIUM_LOW_FPS
static int phytium_dplp_hw_get_low_fps_state(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t val = 0;
	uint32_t group_offset = priv->dplp_reg_base[port];

	val = phytium_dplp_read_reg(priv, group_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_STATE);

	if ((val&LOWFPS_AVAILABLE) && !(val&FRAME_CHANGE_IN_WAIT_LOWFPS)) {
		phytium_dp->dplp_frame_compare_state.lowfps_available = true;
		phytium_dp->dplp_frame_compare_state.lowfps_exit = false;
		phytium_dplp_write_reg(priv, LOWFPS_AVAILABLE, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		FTD330_LOG("get LOWFPS_AVAILABLE irq\n");
		return 1;
	} else if (val & LOWFPS_EXIT) {
		phytium_dp->dplp_frame_compare_state.lowfps_exit = true;
		phytium_dp->dplp_frame_compare_state.lowfps_available = false;
		phytium_dplp_write_reg(priv, LOWFPS_EXIT, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		FTD330_LOG("get LOWFPS_EXIT irq\n");
		return 1;
	}
	return 0;
}
#endif
static void phytium_dp_hw_get_hpd_state(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t val = 0, raw_state = 0;
	uint32_t group_offset = priv->dp_reg_base[port];

	val = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_RAW_STATUS);

	/* maybe miss hpd, so used for clear PHYTIUM_DP_INTERRUPT_RAW_STATUS */
	phytium_readl_reg(priv, group_offset, PHYTIUM_DP_INTERRUPT_STATUS);
	raw_state = phytium_readl_reg(priv, group_offset, PHYTIUM_DP_SINK_HPD_STATE);
	if (val & HPD_EVENT) {
		phytium_dp->dp_hpd_state.hpd_event_state = true;
		FTD330_LOG("dp_%d hpd evnet\n",phytium_dp->port);
	}

	if (val & HPD_IRQ) {
		phytium_dp->dp_hpd_state.hpd_irq_state = true;
		FTD330_LOG("dp_%d hpd irq\n",phytium_dp->port);
	}

	if (raw_state & HPD_CONNECT) {
		phytium_dp->dp_hpd_state.hpd_raw_state = true;
	} else {
		phytium_dp->dp_hpd_state.hpd_raw_state = false;
	}

		FTD330_LOG("dp_%d hpd raw state = %d\n",phytium_dp->port, phytium_dp->dp_hpd_state.hpd_raw_state);
}

static void phytium_dplp_hw_psr_irq_setup(struct phytium_dp_device *phytium_dp, bool enable)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t val;

	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);
	if (enable) {
		phytium_dplp_write_reg(priv, PSR_AVAILABLE | LOWFPS_AVAILABLE, dplp_offset,
					PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		val = val ? (val&PSR_AVAILABLE_MASK) : PSR_AVAILABLE_MASK;
		val &= LOW_FPS_ENABLE_MASK;
		phytium_dplp_write_reg(priv, val, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);
	}
	else {
		val |= (PSR_SHIELD_MASK | LOWFPS_AVAILABLE_MASK);
		phytium_dplp_write_reg(priv, val, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);
	}
}

static void phytium_dp_hpd_poll_handler(struct ftd330_drm_private *priv)
{
	struct drm_device *dev = priv->drm_dev;
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *connector;
	enum drm_connector_status old_status;
	bool changed = false;

	FTD330_LOG_TRACE;
	mutex_lock(&dev->mode_config.mutex);
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

static void phytium_get_missed_interrupt(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	bool handle = false;
	unsigned long irq_flag;
	struct drm_encoder *encoder;
	bool hpd_raw_state_old = false;

	if (phytium_dp->is_edp)
		return;
	
	FTD330_LOG_TRACE;
	
	spin_lock_irqsave(&priv->hotplug_irq_lock, irq_flag);

	drm_for_each_encoder(encoder, dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		if (phytium_dp->is_edp) {
			continue;
		}
		hpd_raw_state_old = phytium_dp->dp_hpd_state.hpd_raw_state;
		phytium_dp_hw_get_hpd_state(phytium_dp);
		if (phytium_dp->dp_hpd_state.hpd_event_state ||
				phytium_dp->dp_hpd_state.hpd_irq_state ||
				hpd_raw_state_old != phytium_dp->dp_hpd_state.hpd_raw_state) {
			handle = true;
		}
	}
	spin_unlock_irqrestore(&priv->hotplug_irq_lock, irq_flag);
	
	if (handle) {
		phytium_dp_hpd_poll_handler(priv);
	}
}
void phytium_dp_hw_hpd_irq_setup(struct phytium_dp_device *phytium_dp, bool enable, bool handle_irq)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	FTD330_LOG_TRACE;
	if (enable) {
		if (phytium_dp && handle_irq) {
			phytium_get_missed_interrupt(phytium_dp);
		}
		phytium_writel_reg(priv, HPD_OTHER_MASK, group_offset, PHYTIUM_DP_INTERRUPT_MASK);
	} else {
		phytium_writel_reg(priv, HPD_IRQ_MASK|HPD_EVENT_MASK|HPD_OTHER_MASK,
				   group_offset, PHYTIUM_DP_INTERRUPT_MASK);
	}
}

static int phytium_dp_hw_init(struct phytium_dp_device *phytium_dp)
{
	int ret = 0;
	struct ftd330_drm_private *priv = phytium_dp->dev->dev_private;

	FTD330_LOG_TRACE;
	phytium_dp->source_rates = phytium_rate;
	phytium_dp->num_source_rates = num_source_rates;

	ret = phytium_dp->funcs->dp_hw_reset(phytium_dp);
	if (ret)
		goto out;
	ret = phytium_dp->funcs->dp_hw_init_phy(phytium_dp);
	if (ret)
		goto out;

	phytium_dp->fast_train_support = false;
	phytium_dp->hw_spread_enable = phytium_dp->funcs->dp_hw_spread_is_enable(phytium_dp);
/*we have to add this after init_phy,or first hpd int will be missed*/
	phytium_dp_registers_init(priv, phytium_dp->port);
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
	if (drm_dp_enhanced_frame_cap(phytium_dp->dpcd))
		link_config[1] |= DP_LANE_COUNT_ENHANCED_FRAME_EN;

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

static void phytium_dp_dpcd_sink_dpms(struct phytium_dp_device *phytium_dp, int mode)
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
	unsigned char voltage, max_phywing_tries;
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
	max_phywing_tries = 0;
	for (;;) {
		unsigned char link_status[DP_LINK_STATUS_SIZE];
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
		drm_dp_link_train_clock_recovery_delay(phytium_dp->dpcd);
#else
		drm_dp_link_train_clock_recovery_delay(&phytium_dp->aux, phytium_dp->dpcd);
#endif
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

		if (max_phywing_tries == 1) {
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
			++max_phywing_tries;

		DRM_DEBUG_KMS("try train_set:0x%x voltage_tries:%d max_phywing_tries:%d\n",
			       phytium_dp->train_set[0], voltage_tries, max_phywing_tries);
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
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
		drm_dp_link_train_channel_eq_delay(phytium_dp->dpcd);
#else
		drm_dp_link_train_channel_eq_delay(&phytium_dp->aux, phytium_dp->dpcd);
#endif
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

static int phytium_dp_get_link_train_fallback_values(struct phytium_dp_device *phytium_dp)
{
	int index, ret = 0;


	if (phytium_dp->is_edp) {
		phytium_dp->train_retry_count++;
		DRM_INFO("Retrying Link training for eDP(%d) with same parameters\n",
			  phytium_dp->port);
		ret = -1;
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
	phytium_dp->max_pass_link_rate = phytium_dp->link_rate;
	phytium_dp->max_pass_lane_count = phytium_dp->link_lane_count;

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

	FTD330_LOG_TRACE;
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

	if (phytium_dp->link_rate > phytium_dp->max_pass_link_rate)
		phytium_dp->max_pass_link_rate = phytium_dp->link_rate;
	if (phytium_dp->link_lane_count > phytium_dp->max_pass_lane_count)
		phytium_dp->max_pass_lane_count = phytium_dp->link_lane_count;

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


	ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_TEST_PHY_PATTERN,
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
	bool video_enable = false, audio_enable = false;
	uint32_t index = 0;
	struct edid *edid = NULL;
	int ret = 0;

	FTD330_LOG_TRACE;
	if (phytium_dp->is_edp)
		status = connector_status_connected;
	else if (hpd_raw_state) {
		if (!is_dp_powered(phytium_dp)) {
			pr_err("dp not powered,but need long pulse\n");
			status = connector_status_disconnected;
			goto out;
		}
		if (!phytium_dp_needs_link_retrain(phytium_dp) &&
				(phytium_dp->dpcd[DP_DPCD_REV] != 0)) {
			status = connector_status_connected;
			goto out;
		}
	} else {
		status = connector_status_disconnected;
		goto out;
	}

	edid = drm_get_edid(connector, &phytium_dp->aux.ddc);
	if (!phytium_dp->is_edp) {
		status = phytium_dp_detect_dpcd(phytium_dp);
		if (status == connector_status_disconnected)
			goto out;
		phytium_dp_set_bios_common_rates_lanes(phytium_dp, edid);
		index = phytium_dp->num_common_rates-1;
		phytium_dp->max_link_rate = phytium_dp->common_rates[index];
		phytium_dp->max_link_lane_count = phytium_dp->common_max_lane_count;
		phytium_dp->link_rate = phytium_dp->max_link_rate;
		phytium_dp->link_lane_count = phytium_dp->max_link_lane_count;
		DRM_DEBUG_KMS("common_max_lane_count: %d, common_max_rate:%d\n",
			       phytium_dp->max_link_lane_count, phytium_dp->max_link_rate);

		video_enable = phytium_dp_hw_video_is_enable(phytium_dp);
		audio_enable = phytium_dp_hw_audio_is_enable(phytium_dp);
		ret = phytium_dp_start_link_train(phytium_dp);

		if (ret < 0)
			goto out;

		if (video_enable) {
			mdelay(2);
			phytium_dp_hw_enable_video(phytium_dp);
		}

		if (audio_enable) {
			mdelay(2);
			phytium_dp_hw_enable_audio(phytium_dp);
		}

		if (edid && drm_edid_is_valid(edid))
			phytium_dp->has_audio = drm_detect_monitor_audio(edid);
		else
			phytium_dp->has_audio = false;
		if (edid)
			kfree(edid);
	}

out:
	return status;
}

static int phytium_dp_short_pulse(struct drm_connector *connector, enum drm_connector_status old_status)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	enum drm_connector_status status = old_status;
	u8 sink_irq_vector = 0;
	bool video_enable = false, audio_enable = false;
	int ret = 0;

	FTD330_LOG_TRACE;
	if (!is_dp_powered(phytium_dp)) {
			pr_err("dp not powered,but need shrot pulse\n");
			status = connector_status_disconnected;
			goto out;
	}
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

#ifdef CONFIG_PHYTIUM_PSR
	mutex_lock(&phytium_dp->low_power_mutex);
	phytium_psr_short_pulse(phytium_dp);
	mutex_unlock(&phytium_dp->low_power_mutex);
#endif

	if (!phytium_dp_needs_link_retrain(phytium_dp)) {
		status = connector_status_connected;
		goto out;
	}

	video_enable = phytium_dp_hw_video_is_enable(phytium_dp);
	audio_enable = phytium_dp_hw_audio_is_enable(phytium_dp);
	ret = phytium_dp_start_link_train(phytium_dp);

	if (ret < 0)
		goto out;

	if (video_enable) {
		mdelay(2);
		phytium_dp_hw_enable_video(phytium_dp);
	}

	if (audio_enable) {
		mdelay(2);
		phytium_dp_hw_enable_audio(phytium_dp);
	}

out:
	return status;
}

void phytium_dp_hpd_irq_setup(struct drm_device *dev, bool enable, bool handle_irq)
{
	struct phytium_dp_device *phytium_dp;
	struct drm_encoder *encoder;
	
	FTD330_LOG_TRACE;
	drm_for_each_encoder(encoder, dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		phytium_dp_hw_hpd_irq_setup(phytium_dp, enable, handle_irq);
	}
}

#ifndef CONFIG_PHYTIUM_PCIE
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
static void
phytium_display_power_request_acpi(struct ftd330_drm_private *priv, bool enable, int display_id)
{
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	acpi_handle handle = ACPI_HANDLE(dev);
	union acpi_object args[3];
	struct acpi_object_list arg_list = {
		.pointer = args,
		.count = ARRAY_SIZE(args),
	};
	acpi_status status;
	long long ret;


	args[0].type = ACPI_TYPE_INTEGER;
	args[0].integer.value = 2;
	args[1].type = ACPI_TYPE_INTEGER;
	args[1].integer.value = display_id;
	args[2].type = ACPI_TYPE_INTEGER;
	args[2].integer.value = 0x0;
	if (!has_acpi_companion(dev))
		pr_err("get acpi device failed\n");
	if (enable) {
		status = acpi_evaluate_integer(handle, "PPWO", &arg_list, &ret);
		if (ACPI_FAILURE(status)) {
			pr_err("No PS0 Method\n");
			return;
		}
		if (ret < 0) {
			pr_err("Failed to suspend");
			return;
		}
	} else {
		status = acpi_evaluate_integer(handle, "PPWD", &arg_list, &ret);
		if (ACPI_FAILURE(status)) {
			pr_err("No PS3 Method\n");
			return;
		}
		if (ret < 0) {
			pr_err("Failed to resume");
			return;
		}
	}
	pr_info("FTD330 acpi power operation succeed\n");
}
#endif
#endif

#ifdef CONFIG_PHYTIUM_POWER_OPERATION
void phytium_display_power_request(struct ftd330_drm_private *priv, bool enable, int display_id)
{
#ifdef CONFIG_PHYTIUM_PCIE
	phytium_display_power_request_se(priv, enable, display_id);
#else
	struct platform_device *pdev = priv->pdev;

	FTD330_LOG_TRACE;
	if (pdev->dev.of_node)
		phytium_display_power_request_se(priv, enable, display_id);
	else if (has_acpi_companion(&pdev->dev))
		phytium_display_power_request_acpi(priv, enable, display_id);
#endif

}
#endif


void phytium_power_off_extra_display(struct ftd330_drm_private *priv)
{
	int i = 0;
	uint32_t group_offset;
	struct device *dev = &priv->pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);

	for (i = 0; i < DISPLAY_NUM;i++) {
		group_offset = PHYTIUM_FTD330_DP_REG_OFFSET + i * PHYTIUM_FTD330_DP_REG_INTERVAL;
		if (!priv->phytium_dp[i] || (priv->phytium_dp[i]->connector.status == connector_status_disconnected && !(priv->info.edp_mask & BIT(i)))) {
				phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
				dc_hw_stop_trigger(&dc->hw, i);
				phytium_display_power_request_off(priv->drm_dev, i);
		}				
	}
}

void phytium_dp_hpd_work_func(struct work_struct *work)
{
	struct ftd330_drm_private *priv =
		container_of(work, struct ftd330_drm_private, hotplug_work);
	struct drm_device *dev = priv->drm_dev;
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *connector;
	enum drm_connector_status old_status;
	bool changed = false;

	FTD330_LOG_TRACE;

	mutex_lock(&dev->mode_config.mutex);
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(connector, &conn_iter) { //遍历每一个connector
		if (!connector->force) {
			old_status = connector->status;
			connector->status = drm_helper_probe_detect(connector, NULL, false);
			if (old_status != connector->status) {
				const char *old, *new;

				old = drm_get_connector_status_name(old_status);
				new = drm_get_connector_status_name(connector->status);
				pr_info("[CONNECTOR:%d:%s] status updated from %s to %s\n",
					connector->base.id,
					connector->name,
					old, new);
				changed = true;
			}
		}
	}
	drm_connector_list_iter_end(&conn_iter);
	mutex_unlock(&dev->mode_config.mutex);

	if (changed)
		drm_kms_helper_hotplug_event(dev);

	phytium_dp_hpd_irq_setup(dev, true, true);

}

void phytium_display_power_request_on(struct drm_device *dev, int physical_display_id, bool handle_irq)
{
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_dp_device *phytium_dp = NULL;
	bool power_status;
	FTD330_LOG_TRACE;

	if (physical_display_id == DISPLAY_0) {
		mutex_lock(&priv->power_mutex);
		power_status = true;
		if (priv->info.pipe_mask & BIT(DISPLAY_0) &&
			!(priv->info.edp_mask & BIT(DISPLAY_0))) {
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
			phytium_dp = priv->phytium_dp[physical_display_id];
			if (!phytium_dp) {
				pr_err("DISPLAY_%d phytium_dp get fail,power on skip\n",physical_display_id);
			}
			power_status = is_dp_powered(phytium_dp);
			if (!power_status) {
			phytium_display_power_request(priv, true, DC_0);
				phytium_dc_registers_init(priv, DC_0);
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
					phytium_dp_hw_init(phytium_dp);
#endif
					phytium_dplp_init_port(phytium_dp);
			}
		}
		mutex_unlock(&priv->power_mutex);
		if (!power_status) {
			phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_0], true, handle_irq);
		}
#endif
	} else {
		mutex_lock(&priv->power_mutex);
		power_status = true;
		if (!(priv->info.edp_mask & BIT(DISPLAY_1)) &&
					!(priv->info.edp_mask & BIT(DISPLAY_2))) {
			if (priv->info.pipe_mask & BIT(DISPLAY_1)) {
				phytium_dp = priv->phytium_dp[DISPLAY_1];
				if (!phytium_dp) {
					pr_err("DISPLAY_%d phytium_dp get fail,power on skip\n",DISPLAY_1);
				}
				power_status = is_dp_powered(phytium_dp);
			}
			if (priv->info.pipe_mask & BIT(DISPLAY_2)) {
				phytium_dp = priv->phytium_dp[DISPLAY_2];
				if (!phytium_dp) {
					pr_err("DISPLAY_%d phytium_dp get fail,power on skip\n",DISPLAY_2);
				}
				power_status = is_dp_powered(phytium_dp);
			}
		}
		if (!(priv->info.pipe_mask & BIT(DISPLAY_1)) && 
					!(priv->info.pipe_mask & BIT(DISPLAY_2))) {
				pr_err("DISPLAY_1 and DISPLAY_2 not used,why power up?\n");
				power_status = true;
		}
		if (!power_status) {
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
		phytium_display_power_request(priv, true, DC_1);
			phytium_dc_registers_init(priv, DC_1);
			if (priv->info.pipe_mask & BIT(DISPLAY_1)) {
				phytium_dp = priv->phytium_dp[DISPLAY_1];
				phytium_dp_hw_init(phytium_dp);
				phytium_dplp_init_port(phytium_dp);
			}
			if (priv->info.pipe_mask & BIT(DISPLAY_2)) {
				phytium_dp = priv->phytium_dp[DISPLAY_2];
				phytium_dp_hw_init(phytium_dp);
				phytium_dplp_init_port(phytium_dp);
			}
#endif
		}
		mutex_unlock(&priv->power_mutex);
		if (!power_status) {
			if (priv->info.pipe_mask & BIT(DISPLAY_1)) {
				phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_1], true, handle_irq);
			}
			if (priv->info.pipe_mask & BIT(DISPLAY_2)) {
				phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_2], true, handle_irq);
			}
		}
	}
}


void phytium_display_power_request_off(struct drm_device *dev, int physical_display_id)
{
	struct ftd330_drm_private *priv = dev->dev_private;
	enum drm_connector_status connector_status[DISPLAY_NUM];
	int i = 0;
	FTD330_LOG_TRACE;
	mutex_lock(&priv->power_mutex);

	for (i = 0;i < DISPLAY_NUM;i++) {
	     if (priv->phytium_dp[i]) {
	         connector_status[i] = priv->phytium_dp[i]->connector.status;
         } else {
             connector_status[i] = connector_status_disconnected;
         }
    }

	if (physical_display_id == DISPLAY_0) {
		if (!(priv->info.edp_mask & BIT(DISPLAY_0)) && (connector_status[DISPLAY_0] != connector_status_connected)) {
			if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
				phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_0], false, false);
			}
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
			phytium_display_power_request(priv, false, DC_0);
#endif
		}
	} else {
		if(!(priv->info.edp_mask & BIT(DISPLAY_1)) && !(priv->info.edp_mask & BIT(DISPLAY_2))
								&& (connector_status[DISPLAY_1] != connector_status_connected)
								&& (connector_status[DISPLAY_2] != connector_status_connected)) {
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
			if ((physical_display_id == DISPLAY_1) &&
				!(priv->info.pipe_mask & BIT(DISPLAY_2))) {
				phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_1], false, false);
				phytium_display_power_request(priv, false, DC_1);
			}

			if ((physical_display_id == DISPLAY_2) &&
				!(priv->info.pipe_mask & BIT(DISPLAY_1))) {
				phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_2], false, false);
				phytium_display_power_request(priv, false, DC_1);
			}

			if (priv->info.pipe_mask & BIT(DISPLAY_1) && priv->info.pipe_mask & BIT(DISPLAY_2)) {
				priv->need_power_down[physical_display_id] = true;

				if (priv->need_power_down[DISPLAY_1] && priv->need_power_down[DISPLAY_2]) {
					phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_1], false, false);
					phytium_dp_hw_hpd_irq_setup(priv->phytium_dp[DISPLAY_2], false, false);
					phytium_display_power_request(priv, false, DC_1);
					priv->need_power_down[DISPLAY_1] = false;
					priv->need_power_down[DISPLAY_2] = false;
				}
			}
#endif
		}
	}
	mutex_unlock(&priv->power_mutex);
}


void phytium_dp_power_work_func(struct work_struct *work)
{
	struct phytium_dp_device *phytium_dp =
		container_of(work, struct phytium_dp_device, power_work);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int temp_port = 0;

	FTD330_LOG_TRACE;
	if (!priv->low_power_enable[phytium_dp->port]) {
		pr_info("FTD330 low_power disabled,no power operation\n");
		return;
	}

	temp_port = phytium_port_virtual_to_physical(phytium_dp);

	phytium_display_power_request_on(dev, temp_port, true);




				

	//pr_err("phytium_dp-%d in func:%s line:%d\n",phytium_dp->port, __func__, __LINE__);
	/*pr_info("phytium_dp->port = %d,present_dp_power_status is %d,%d,%d", phytium_dp->port,
                        priv->power_status[DISPLAY_0], priv->power_status[DISPLAY_1],
                        priv->power_status[DISPLAY_2]);*/

}

#ifdef CONFIG_PHYTIUM_PSR
static void phytium_dp_psr_work_func(struct work_struct *work)
{
	struct phytium_dp_device *phytium_dp =
		container_of(work, struct phytium_dp_device, psr_work);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t group_offset = priv->dplp_reg_base[phytium_dp->port];

	FTD330_LOG_TRACE;
	mutex_lock(&phytium_dp->low_power_mutex);
	if (phytium_dp->dplp_frame_compare_state.psr_available == true) {
		phytium_dplp_write_reg(priv, ENTER_PSR_REQ, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_REQ);
		phytium_psr_enable(phytium_dp);
#ifdef CONFIG_PHYTIUM_PSR_SF_UPDATE
	} else if (phytium_dp->dplp_frame_compare_state.frame_change_in_psr == true) {
		phytium_psr_exit_sf_detect(phytium_dp);
	}

	if (phytium_dp->dplp_frame_compare_state.psr_exit == true) {
		FTD330_LOG("exit psr\n");
		phytium_dp->psr.frame_change_irq_nums = 0;
		phytium_psr_disable(phytium_dp);
		phytium_dplp_write_reg(priv, EXIT_PSR_REQ, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_REQ);
	} else {
		phytium_psr_sf_update(phytium_dp);
	}
#else
	} else if (phytium_dp->dplp_frame_compare_state.frame_change_in_psr == true) {
		FTD330_LOG("get frame_change_in_psr irq\n");
		phytium_psr_disable(phytium_dp);
		phytium_dplp_write_reg(priv, EXIT_PSR_REQ, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_REQ);
		phytium_dplp_write_reg(priv, PSR_AVAILABLE|FRAME_CHANGE_IN_PSR, group_offset,
						PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
		phytium_dplp_hw_psr_irq_setup(phytium_dp, true);
	}
#endif
	mutex_unlock(&phytium_dp->low_power_mutex);
}
#endif

#ifdef CONFIG_PHYTIUM_LOW_FPS
static void phytium_dp_low_fps_work_func(struct work_struct *work)
{
	struct phytium_dp_device *phytium_dp =
		container_of(work, struct phytium_dp_device, low_fps_work);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t group_offset = priv->dplp_reg_base[phytium_dp->port];

	FTD330_LOG_TRACE;
	mutex_lock(&phytium_dp->low_power_mutex);
	if (phytium_dp->dplp_frame_compare_state.lowfps_available == true) {
		phytium_dplp_write_reg(priv, ENTER_LOWFPS_REQ, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_REQ);
		phytium_change_fps(phytium_dp, true, 0);
	} else if (phytium_dp->dplp_frame_compare_state.lowfps_exit == true) {
		phytium_change_fps(phytium_dp, false, 0);
		phytium_dplp_write_reg(priv, EXIT_LOWFPS_REQ, group_offset,
								PHYTIUM_DPLP_FRAME_COMP_REQ);
	}
	mutex_unlock(&phytium_dp->low_power_mutex);
}
#endif

irqreturn_t phytium_dp_hpd_irq_handler(int irq, void *data)
{
	struct phytium_dp_device *phytium_dp = data;
	struct phytium_dp_device *temp_phytium_dp = NULL;
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct drm_encoder *encoder = NULL;
#ifdef CONFIG_PHYTIUM_WRITEBACK
	pr_info("Using writeback Connector,no hotplug event handled\n");
	return IRQ_HANDLED;
#endif
	bool changed = false;
	bool psr_available = false;
	bool psr_changed = false;
	unsigned long irq_flag;
#ifdef CONFIG_PHYTIUM_LOW_FPS
	bool lowfps_changed = false;
#endif

	FTD330_LOG_TRACE;

	spin_lock_irqsave(&priv->hotplug_irq_lock, irq_flag);
	drm_for_each_encoder(encoder, dev) {
			temp_phytium_dp = encoder_to_dp_device(encoder);
			phytium_dp_hw_get_hpd_state(temp_phytium_dp);
			if (temp_phytium_dp->dp_hpd_state.hpd_event_state
			|| temp_phytium_dp->dp_hpd_state.hpd_irq_state) {
		changed = true;
			}
	}
#ifdef CONFIG_PHYTIUM_PSR
	psr_changed = phytium_dplp_hw_get_psr_state(phytium_dp);
	if (phytium_dp->dplp_frame_compare_state.psr_available == true)
		psr_available = true;
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
	lowfps_changed = phytium_dplp_hw_get_low_fps_state(phytium_dp);
#endif
	spin_unlock_irqrestore(&priv->hotplug_irq_lock,irq_flag);

	if (psr_available && psr_changed)
		phytium_dplp_hw_psr_irq_setup(phytium_dp, false);

	if (!phytium_dp->is_edp && changed) {
		phytium_dp_hpd_irq_setup(dev, false, false);
		schedule_work(&priv->hotplug_work);

	}

#ifdef CONFIG_PHYTIUM_PSR
	if (phytium_dp->dplp_frame_compare_state.psr_available == true &&
			psr_changed)
		schedule_work(&phytium_dp->psr_work);
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
	if ((phytium_dp->dplp_frame_compare_state.lowfps_available == true ||
				phytium_dp->dplp_frame_compare_state.lowfps_exit == true) &&
				lowfps_changed) {
		schedule_work(&phytium_dp->low_fps_work);
	}
#endif
	return IRQ_HANDLED;
}

irqreturn_t phytium_dp_power_on_irq_handler(int irq, void *data)
{
	struct phytium_dp_device *phytium_dp = data;
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t group_offset = priv->dplp_reg_base[phytium_dp->port];
	uint32_t status;

	FTD330_LOG_TRACE;
#ifdef CONFIG_PHYTIUM_WRITEBACK
	pr_info("Using writeback Connector,no hotplug event handled\n");
	return IRQ_HANDLED;
#endif

	status = phytium_dplp_read_reg(priv, group_offset, 0x4);
        phytium_dplp_write_reg(priv, CLEAN_HPD_CONNECT, group_offset,
                                          PHYTIUM_DPLP_HPD_PWRUP_INT_CLEAR);
	if (!(status & 0x01)) {
		return IRQ_HANDLED;
	}

	phytium_dplp_deinit_port(phytium_dp);
	schedule_work(&phytium_dp->power_work);
	return IRQ_HANDLED;
}


#ifdef CONFIG_PHYTIUM_LANE_TRAIN
void phytium_dp_fast_link_train_detect(struct phytium_dp_device *phytium_dp)
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
#endif

static enum drm_connector_status
phytium_connector_detect(struct drm_connector *connector, bool force)
{
/*we do link train here.so we can */
	enum drm_connector_status status = connector->status;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	bool hpd_event_state, hpd_irq_state, hpd_raw_state, plugged;
	unsigned long irq_flag;
	struct drm_device *drm_dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;

	FTD330_LOG_TRACE;
	spin_lock_irqsave(&priv->hotplug_irq_lock, irq_flag);
	hpd_event_state = phytium_dp->dp_hpd_state.hpd_event_state;
	hpd_irq_state = phytium_dp->dp_hpd_state.hpd_irq_state;
	hpd_raw_state = phytium_dp->dp_hpd_state.hpd_raw_state;
	phytium_dp->dp_hpd_state.hpd_event_state = false;
	phytium_dp->dp_hpd_state.hpd_irq_state = false;
	spin_unlock_irqrestore(&priv->hotplug_irq_lock,irq_flag);

	if (hpd_event_state)
		status = phytium_dp_long_pulse(connector, hpd_raw_state);
	
	if (!phytium_dp->is_edp && hpd_irq_state)
		status = phytium_dp_short_pulse(connector, status);

	if (status == connector_status_unknown)
		status = connector_status_disconnected;

	if ((!phytium_dp->is_edp) && (!hpd_raw_state))
		status = connector_status_disconnected;
	if (phytium_dp->is_edp) {
		status = connector_status_connected;
	}

	if (connector->status != status) {
		if ((status == connector_status_connected) && phytium_dp->has_audio)
			plugged = true;
		else
			plugged = false;

		handle_plugged_change(phytium_dp, plugged);
	}

	if (status == connector_status_connected) {
		FTD330_LOG("dp_%d connected\n",phytium_dp->port);
	} else {
		FTD330_LOG("dp_%d disconnected\n",phytium_dp->port);
	}

	if (connector->status != status) {
		if (status == connector_status_disconnected) {
			phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, false);
		} else {
			phytium_dp->funcs->dp_hw_config_phy_power(phytium_dp, true);
		}
	}

	return status;
}

static void
phytium_connector_destroy(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;

	cancel_work_sync(&priv->hotplug_work);
	if (phytium_dp->is_edp) {
#ifdef CONFIG_PHYTIUM_PSR
		if (phytium_dp->psr.psr_work_init) {
			cancel_work_sync(&phytium_dp->psr_work);
			phytium_dp->psr.psr_work_init = false;
		}
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
		if (phytium_dp->vrr.lowfps_work_init) {
			cancel_work_sync(&phytium_dp->low_fps_work);
			phytium_dp->vrr.lowfps_work_init = false;
		}
#endif
	}

	if (!list_empty(&phytium_dp->phytium_mode.list))
		phytium_dp_clean_display_modes(phytium_dp);

	drm_connector_cleanup(connector);
	if (phytium_dp)
		kfree(phytium_dp);
}

static int
phytium_dp_connector_register(struct drm_connector *connector)
{
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	int ret = 0;
#ifndef CONFIG_PHYTIUM_PCIE
#if defined(CONFIG_PHYTIUM_PSR) || defined(CONFIG_PHYTIUM_LOW_FPS) || defined(CONFIG_PHYTIUM_LANE_TRAIN)
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct platform_device *pdev = priv->pdev;
	int temp_port = phytium_port_virtual_to_physical(phytium_dp);

	/* maybe miss hpd, clear PHYTIUM_DP_INTERRUPT_RAW_STATUS before enable interrupt*/

	ret = request_irq(platform_get_irq(pdev, (temp_port + 2)),
						phytium_dp_hpd_irq_handler, IRQF_SHARED,
						dev_name(&pdev->dev), phytium_dp);//下电中断
	if (ret < 0) {
		pr_info("Failed to install irq\n");
		return ret;
	}

	ret = request_irq(platform_get_irq(pdev, (temp_port + 5)),
					phytium_dp_power_on_irq_handler, IRQF_SHARED,
					dev_name(&pdev->dev), phytium_dp);//上电中断
	if (ret < 0) {
		pr_info("Failed to install irq\n");
		return ret;
	}
#endif
#endif

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	if (phytium_dp->is_edp) {
		phytium_edp_panel_poweron(phytium_dp);
		ret = phytium_edp_backlight_device_register(phytium_dp);
		if (ret)
			DRM_ERROR("failed to register port(%d) backlight device(ret=%d)\n",
				phytium_dp->port, ret);
		phytium_edp_init_connector(phytium_dp);
	}
#endif

#ifdef CONFIG_DEBUG_FS
	ret = phytium_dp_debugfs_connector_add(connector);
	if (ret)
		DRM_ERROR("failed to register phytium connector debugfs(ret=%d)\n", ret);
#endif

	return 0;
}

static void
phytium_dp_connector_unregister(struct drm_connector *connector)
{
#if (!defined(CONFIG_PHYTIUM_PCIE) && \
	defined(CONFIG_PHYTIUM_LANE_TRAIN)) || \
	defined(CONFIG_PHYTIUM_EDP_BL) || \
	defined(CONFIG_PHYTIUM_LANE_TRAIN)
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
#endif
#ifndef CONFIG_PHYTIUM_PCIE
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct platform_device *pdev = priv->pdev;
	int temp_port = phytium_port_virtual_to_physical(phytium_dp);

	free_irq(platform_get_irq(pdev, (temp_port + 2)), phytium_dp);
	free_irq(platform_get_irq(pdev, (temp_port + 5)), phytium_dp);
#endif
#endif

#ifdef CONFIG_PHYTIUM_EDP_BL
	if (phytium_dp->is_edp) {
		phytium_edp_backlight_device_unregister(phytium_dp);
		phytium_edp_fini_connector(phytium_dp);
	}
#endif

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	drm_dp_aux_unregister(&phytium_dp->aux);
#endif
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
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);

	FTD330_LOG("%s_%d: crtc_hdisplay:%d  %d\n", __func__, __LINE__,
			mode->crtc_hdisplay, adjusted->crtc_hdisplay);

	/*
	 * this func was called before dc_check when S3 return
	 * (screen off-> S3 ->return) on kylin.So incase ajusted
	 * was not set,we set it here to avoid bad dp configuration
	 */
	phytium_update_timing_for_drm_display_mode(adjusted, &phytium_dp->native_mode);
	drm_mode_copy(&phytium_dp->mode, adjusted);
}
#ifdef CONFIG_PHYTIUM_EDP_BL
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
#endif
static void phytium_encoder_disable(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);
#if defined(CONFIG_PHYTIUM_LOW_FPS) || defined(CONFIG_PHYTIUM_PSR)
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dplp_reg_base[port];
	
#endif
#ifdef CONFIG_PHYTIUM_PSR
	bool psr_is_enabled = false;
#endif
#if defined(CONFIG_PHYTIUM_LOW_FPS)
	bool lowfps_is_enabled = false;
#endif

	FTD330_LOG_TRACE;
	if (phytium_dp->is_edp) {
#ifdef CONFIG_PHYTIUM_PSR
		mutex_lock(&phytium_dp->low_power_mutex);
		psr_is_enabled = phytium_dp->psr.enabled;
		if (psr_is_enabled) {
			phytium_dp->dplp_frame_compare_state.psr_available = false;
			phytium_dp->dplp_frame_compare_state.frame_change_in_psr = true;
			phytium_psr_disable(phytium_dp);
			phytium_dplp_write_reg(priv, EXIT_PSR_REQ, group_offset,
									PHYTIUM_DPLP_FRAME_COMP_REQ);
		}
		mutex_unlock(&phytium_dp->low_power_mutex);
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
		mutex_lock(&phytium_dp->low_power_mutex);
		lowfps_is_enabled = phytium_dp->vrr.enabled;
		if (lowfps_is_enabled) {
			phytium_dp->dplp_frame_compare_state.lowfps_exit = true;
			phytium_dp->dplp_frame_compare_state.lowfps_available = false;
			phytium_change_fps(phytium_dp, false, 0);
			phytium_dplp_write_reg(priv, EXIT_LOWFPS_REQ, group_offset,
									PHYTIUM_DPLP_FRAME_COMP_REQ);
		}
		mutex_unlock(&phytium_dp->low_power_mutex);
#endif
#if defined(CONFIG_PHYTIUM_LOW_FPS) || defined(CONFIG_PHYTIUM_PSR)
		phytium_psr_lowfps_dplp_deinit(phytium_dp);
#endif
	}

#ifdef CONFIG_PHYTIUM_EDP_BL
	if (phytium_dp->is_edp)
		phytium_edp_backlight_off(phytium_dp);
#endif
	phytium_dp_hw_disable_video(phytium_dp);

	mdelay(50);
#ifdef CONFIG_PHYTIUM_EDP_BL
	if (phytium_dp->is_edp)
		phytium_edp_panel_poweroff(phytium_dp);
#endif


}

static void phytium_dp_adjust_link_train_parameter(struct phytium_dp_device *phytium_dp)
{
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	struct phytium_display_mode *phytium_mode = NULL;
	unsigned long link_bw, date_rate = 0, bs_limit, bs_request;
	int rate = 0;

	phytium_dp->bpc = display_info->bpc;
	list_for_each_entry(phytium_mode, &phytium_dp->phytium_mode.list, list) {
		if (phytium_display_mode_compare(phytium_mode, &phytium_dp->mode) &&
			phytium_mode->reduced_bpc) {
				switch (phytium_mode->bpc) {
				case 10:
					phytium_dp->bpc = 10;
					break;
				case 6:
					phytium_dp->bpc = 6;
					break;
				default:
					phytium_dp->bpc = 8;
					break;
				}
			}
	}

	bs_request = phytium_dp->mode.crtc_htotal/(phytium_dp->mode.crtc_clock/1000);
	date_rate = (phytium_dp->mode.crtc_clock * phytium_dp->bpc * 3)/8;

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

		if (phytium_dp_get_link_train_fallback_values(phytium_dp))
			break;
	}

	DRM_DEBUG_KMS("Try link training at Link Rate = %d, Lane count = %d\n",
		       phytium_dp->link_rate, phytium_dp->link_lane_count);
}

static void phytium_encoder_enable(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);
	int ret = 0;
#ifndef CONFIG_PHYTIUM_LANE_TRAIN
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t group_offset = priv->dp_reg_base[phytium_dp->port];
#endif

	FTD330_LOG_TRACE;

	/* we need skip encoder_enable to avoid DP send message to a 
	 * disconnected monitor, which causes aux read error.
	 */
	if (phytium_dp->connector.status == connector_status_disconnected) {
		return;
	}

	phytium_dp_hw_disable_video(phytium_dp);

	if (phytium_dp->is_edp) {
		phytium_edp_panel_poweron(phytium_dp);
		if (phytium_dp->fast_train_support)
			ret = phytium_dp_fast_link_train(phytium_dp);
		if (!ret)
			ret = phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
		phytium_dp_fast_link_train_detect(phytium_dp);
	} else {
		phytium_dp_adjust_link_train_parameter(phytium_dp);
		ret = phytium_dp_start_link_train(phytium_dp);
		mdelay(2);
	}

	phytium_dp_hw_config_video(phytium_dp);
	phytium_dp_hw_enable_video(phytium_dp);
#ifdef CONFIG_PHYTIUM_PSR
	if(phytium_dp->is_edp) {
		if (phytium_dp->psr.sink_support)
			phytium_psr_dplp_init(phytium_dp);
	}
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
	if(phytium_dp->is_edp) {
		if (phytium_dp->vrr.sink_support)
			phytium_low_fps_dplp_init(phytium_dp);
	}
#endif
		if (phytium_dp->has_audio)
			phytium_dp_hw_enable_audio(phytium_dp);

#ifdef CONFIG_PHYTIUM_EDP_BL
	if (phytium_dp->is_edp)
		phytium_edp_backlight_on(phytium_dp);
#endif

}


enum drm_mode_status
phytium_encoder_mode_valid(struct drm_encoder *encoder, const struct drm_display_mode *mode)
{

	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);
	struct drm_display_info *display_info = &phytium_dp->connector.display_info;
	unsigned int requested, actual;
	enum drm_mode_status ret;
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_display_mode *phytium_mode = NULL;
	int requested_bpc;

	phytium_mode = kzalloc(sizeof(*phytium_mode), GFP_KERNEL);

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
	requested_bpc = display_info->bpc;

	if ((display_info->color_formats & DRM_COLOR_FORMAT_RGB444) == 0) {
		DRM_INFO("not support color_format(%d)\n", display_info->color_formats);
		display_info->color_formats = DRM_COLOR_FORMAT_RGB444;
	}

    if (mode->hdisplay * mode->vdisplay > 3840 * 2160)
            return MODE_BAD_HVALUE;

    if (phytium_dp->port > 0 &&
            priv->info.overlay_enable &&
            mode->hdisplay == 3840 &&
            mode->vdisplay == 2160 &&
            drm_mode_vrefresh(mode) > 30) {

            return MODE_BAD_HVALUE;
    }

    if (mode->clock > 653000) {
    	return MODE_BAD_HVALUE;
    }

	if (phytium_dp->is_edp)
		actual = phytium_dp->max_link_rate * phytium_dp->max_link_lane_count / 100;
	else
		actual = phytium_dp->max_pass_link_rate * phytium_dp->max_pass_lane_count / 100;
	actual = actual * 8 / 10;
	do {
		requested = mode->clock * requested_bpc * 3 / 1000;
		if (requested >= actual)
				requested_bpc -= 2;
	} while ((requested >= actual) && requested_bpc >= 8);

	if (requested_bpc < 8 || (requested >= actual)) {
		DRM_DEBUG_KMS("Mode %dx%d-%dHz (clock=%d) requested=%d more than actual=%d\n",
						mode->hdisplay, mode->vdisplay, drm_mode_vrefresh(mode),
						mode->clock, requested, actual);
		return MODE_CLOCK_HIGH;
	}

	else if (requested_bpc != display_info->bpc) {
		if (!phytium_drm_mode_in_phytium_mode(phytium_dp, mode)) {
			phytium_display_mode_copy(phytium_mode, mode);
			phytium_mode->bpc = requested_bpc;
			phytium_mode->reduced_bpc = true;
			list_add_tail(&phytium_mode->list, &phytium_dp->phytium_mode.list);
			DRM_DEBUG_KMS("Mode %dx%d-%dHz (clk %d) using reduced bpc %d\n",
					mode->hdisplay, mode->vdisplay, drm_mode_vrefresh(mode),
					mode->clock, requested_bpc);
		}
	}

	if (dc_fake_mode_enable &&
		(phytium_dp->native_mode.clock == mode->clock) &&
		(phytium_dp->native_mode.htotal == mode->htotal) &&
		(phytium_dp->native_mode.vtotal == mode->vtotal)) {
		ret = MODE_OK;
		goto status_in_total;
	}

	if ((mode->hdisplay == 1600) && (mode->vdisplay == 900)) {
		ret = MODE_BAD_HVALUE;
		goto status_in_total;
	}

	if ((mode->hdisplay == 1024) && (mode->clock > 78000)) {
		ret = MODE_BAD_HVALUE;
		goto status_in_total;
	}

	if ((mode->hdisplay < 640) || (mode->vdisplay < 480)) {
		ret = MODE_BAD_HVALUE;
		goto status_in_total;
	}

	ret = MODE_OK;

status_in_total:
	return ret;

}

static int phytium_encoder_atomic_check(struct drm_encoder *encoder,
                                   struct drm_crtc_state *crtc_state,
                                   struct drm_connector_state *conn_state)
{
	struct drm_connector *connector = conn_state->connector;
	struct phytium_dp_device *phytium_dp = connector_to_dp_device(connector);
	struct drm_display_info *info = &connector->display_info;
	struct phytium_display_mode *phytium_mode = NULL;
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(crtc_state);
	struct drm_display_mode *mode = &crtc_state->adjusted_mode;
	u32 bus_format;

	switch (info->bpc) {
		case 6:
			bus_format = MEDIA_BUS_FMT_RGB666_1X18;
			break;
		case 8:
			bus_format = MEDIA_BUS_FMT_RGB888_1X24;
			break;
		case 10:
			bus_format = MEDIA_BUS_FMT_RGB101010_1X30;
			break;
		default:
			bus_format = MEDIA_BUS_FMT_RGB888_1X24;
			break;
	}

	list_for_each_entry(phytium_mode, &phytium_dp->phytium_mode.list, list) {
		if (phytium_display_mode_compare(phytium_mode, mode) &&
			phytium_mode->reduced_bpc) {
				switch (phytium_mode->bpc) {
					case 6:
						bus_format = MEDIA_BUS_FMT_RGB666_1X18;
						break;
					case 8:
						bus_format = MEDIA_BUS_FMT_RGB888_1X24;
						break;
					case 10:
						bus_format = MEDIA_BUS_FMT_RGB101010_1X30;
						break;
					default:
						bus_format = MEDIA_BUS_FMT_RGB888_1X24;
						break;
				}
		}
	}

	ftd330_crtc_state->output_fmt = bus_format;
	if (phytium_dp->is_edp) {
		ftd330_crtc_state->encoder_type = DRM_MODE_ENCODER_TMDS;
	} else {
		ftd330_crtc_state->encoder_type = DRM_MODE_ENCODER_DPMST;
	}

    return 0;
}


static const struct drm_encoder_helper_funcs phytium_encoder_helper_funcs = {
	.mode_set = phytium_dp_encoder_mode_set,
	.disable = phytium_encoder_disable,
	.enable  = phytium_encoder_enable,
	.mode_valid = phytium_encoder_mode_valid,
	.atomic_check = phytium_encoder_atomic_check,
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	.digital_mute = phytium_dp_audio_digital_mute,
#else
	.mute_stream = phytium_dp_audio_mute_stream,
#endif
	.get_eld = phytium_dp_audio_get_eld,
	.hook_plugged_cb = phytium_dp_audio_hook_plugged_cb,
};

static int phytium_dp_audio_codec_init(struct phytium_dp_device *phytium_dp)
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
							       codec_id,
							       &codec_data, sizeof(codec_data));

	if (!PTR_ERR_OR_ZERO(phytium_dp->audio_pdev))
		codec_id += 1;

	return PTR_ERR_OR_ZERO(phytium_dp->audio_pdev);
}

static void phytium_dp_audio_codec_fini(struct phytium_dp_device *phytium_dp)
{

	if (!PTR_ERR_OR_ZERO(phytium_dp->audio_pdev))
		platform_device_unregister(phytium_dp->audio_pdev);
	phytium_dp->audio_pdev = NULL;
	codec_id -= 1;
}


static void phytium_dp_encoder_destroy(struct drm_encoder *encoder)
{
	struct phytium_dp_device *phytium_dp = encoder_to_dp_device(encoder);

	phytium_dp_audio_codec_fini(phytium_dp);
	drm_encoder_cleanup(encoder);
}

static const struct drm_encoder_funcs phytium_encoder_funcs = {
	.destroy = phytium_dp_encoder_destroy,
};


//static int phytium_get_encoder_crtc_mask(struct phytium_dp_device *phytium_dp, int port)
//{
//	struct drm_device *dev =  phytium_dp->dev;
//	struct ftd330_drm_private *priv = dev->dev_private;
//	int i, mask = 0;
//
//	for_each_pipe(priv, i) {
//		if (i != port)
//			mask++;
//		else
//			break;
//	}
//
//	return BIT(mask);
//}

static bool phytium_dp_is_edp(struct phytium_dp_device *phytium_dp, int port)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;

	if (priv->info.edp_mask & BIT(port))
		return true;
	else
		return false;
}

static void phytium_edp_init_connector(struct phytium_dp_device *phytium_dp)
{
	FTD330_LOG_TRACE;

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	phytium_dp->max_link_rate = phytium_dp->common_rates[phytium_dp->num_common_rates-1];
	phytium_dp->max_link_lane_count = phytium_dp->common_max_lane_count;
	phytium_dp->link_rate = phytium_dp->max_link_rate;
	phytium_dp->link_lane_count = phytium_dp->max_link_lane_count;
#endif
}
static void phytium_edp_fini_connector(struct phytium_dp_device *phytium_dp)
{
	if (phytium_dp->edp_edid)
		kfree(phytium_dp->edp_edid);
	phytium_dp->edp_edid = NULL;
	return;
}

int phytium_dp_resume(struct drm_device *drm_dev)
{
#if defined(CONFIG_PHYTIUM_EDP_BL) || defined(CONFIG_PHYTIUM_LANE_TRAIN)
	struct phytium_dp_device *phytium_dp;
	struct phytium_panel *panel = NULL;
	struct drm_encoder *encoder;
	int ret = 0;
#endif
#if 0
	bool raw_state;
#endif
	FTD330_LOG_TRACE;

#if defined(CONFIG_PHYTIUM_EDP_BL) || defined(CONFIG_PHYTIUM_LANE_TRAIN)
	drm_for_each_encoder(encoder, drm_dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		ret = phytium_dp_hw_init(phytium_dp);
		if (ret) {
			DRM_ERROR("failed to initialize dp %d\n", phytium_dp->port);
		}

		if (phytium_dp->is_edp) {
			phytium_edp_panel_poweron(phytium_dp);
			if (phytium_dp->panel.setup_backlight) {
				mutex_lock(&phytium_dp->panel.panel_lock);
				phytium_dp->panel.setup_backlight(&phytium_dp->panel);
				mutex_unlock(&phytium_dp->panel.panel_lock);
			} else {
				DRM_ERROR("edp-%d missing setup_backlight func\n", phytium_dp->port);
			}
			panel = &phytium_dp->panel;
			panel->level = panel->save_level;
		}
		if (phytium_dp->audio_info.sample_rate != 0)
			phytium_dp_hw_audio_hw_params(phytium_dp, phytium_dp->audio_info);
	}
#endif
	return 0;
}

int phytium_dp_suspend(struct drm_device *drm_dev)
{
	struct phytium_dp_device *phytium_dp;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct drm_encoder *encoder;
	struct phytium_panel *panel = NULL;
	int real_display_id = 0;
	drm_for_each_encoder(encoder, drm_dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		real_display_id = phytium_display_virtual_to_physical(priv->info.pipe_mask, phytium_dp->port);
		if (phytium_dp->connector.status == connector_status_connected) {
			priv->power_status_save[real_display_id] =  true;
		} else {
			priv->power_status_save[real_display_id] =  false;
		}
		if (phytium_dp->is_edp) {
			panel = &phytium_dp->panel;
			panel->save_level = panel->level;
			phytium_edp_backlight_off(phytium_dp);
			phytium_edp_panel_poweroff(phytium_dp);
		}
		phytium_dp_hw_disable_video(phytium_dp);
	}

	return 0;
}


#if defined(CONFIG_PHYTIUM_LOW_FPS) || defined(CONFIG_PHYTIUM_PSR)
static void phytium_psr_lowfps_dplp_deinit(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t dp_offset = priv->dp_reg_base[port];
	uint32_t val;

	/* mask all irq */
	phytium_dplp_write_reg(priv, ALL_SHIELD_MAKS, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);

	/* disable frame comp*/
	phytium_dplp_write_reg(priv, 0x0, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);

	/* disbale dp crc */
	val = phytium_readl_reg(priv, dp_offset, PHYTIUM_EDP_CRC_ENABLE);
	val &= ~ENABLE_CRC;
	phytium_writel_reg(priv, val, dp_offset, PHYTIUM_EDP_CRC_ENABLE);

	/* clean irq*/
	phytium_dplp_write_reg(priv, 0x3ff, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
}

#endif

#ifdef CONFIG_PHYTIUM_LOW_FPS
static void phytium_low_fps_dplp_init(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t dp_offset = priv->dp_reg_base[port];
	uint32_t val;

	FTD330_LOG_TRACE;
	/* Init dp crc */
	val = phytium_readl_reg(priv, dp_offset, PHYTIUM_EDP_CRC_ENABLE);
	val |= ENABLE_CRC;
	phytium_writel_reg(priv, val, dp_offset, PHYTIUM_EDP_CRC_ENABLE);

	/* config dplp */
	phytium_dplp_write_reg(priv, LOWFPS_CNT_NUMS, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);

	phytium_dplp_write_reg(priv, LOWFPS_AVAILABLE | LOWFPS_EXIT, dplp_offset,
						PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);
	val = val ? (val&LOW_FPS_ENABLE_MASK) : LOW_FPS_ENABLE_MASK;
	phytium_dplp_write_reg(priv, val, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);

	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
	phytium_dplp_write_reg(priv, val | 0x03, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
}
#endif

#ifdef CONFIG_PHYTIUM_PSR
static void phytium_psr_dplp_init(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t dplp_offset = priv->dplp_reg_base[port];
	uint32_t dp_offset = priv->dp_reg_base[port];
	uint32_t val;

	/* Init dp crc */
	val = phytium_readl_reg(priv, dp_offset, PHYTIUM_EDP_CRC_ENABLE);
	val |= ENABLE_CRC;
	phytium_writel_reg(priv, val, dp_offset, PHYTIUM_EDP_CRC_ENABLE);

	/* Config dplp psr */
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);
	val = val ? val : LOWFPS_CNT_NUMS;
	phytium_dplp_write_reg(priv, val, dplp_offset, PHYTIUM_DPLP_LOWFPS_CNT);
	phytium_dplp_write_reg(priv, PSR_CNT_NUMS, dplp_offset, PHYTIUM_DPLP_PSR_CNT);

	/* Rely only on PSR_AVAILABLE interrupts*/
	phytium_dplp_write_reg(priv, PSR_AVAILABLE, dplp_offset,
						PHYTIUM_DPLP_FRAME_COMP_INIT_CLEAR);
	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);
	val = val ? (val&PSR_AVAILABLE_MASK) : PSR_AVAILABLE_MASK;
	phytium_dplp_write_reg(priv, val, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_INIT_MASK);

	val = phytium_dplp_read_reg(priv, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
	phytium_dplp_write_reg(priv, val|0x05, dplp_offset, PHYTIUM_DPLP_FRAME_COMP_CFG);
}
#endif

void phytium_dplp_init_port(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  NULL;
	struct ftd330_drm_private *priv = NULL;
	uint32_t group_offset = 0;

	FTD330_LOG_TRACE;

	if (!phytium_dp) {
		pr_err("phytium_dp not init,skip dplp_deinit\n");
	}
	dev =  phytium_dp->dev;
	priv = dev->dev_private;
	group_offset = priv->dplp_reg_base[phytium_dp->port];
		phytium_dplp_write_reg(priv, 0x06, group_offset, PHYTIUM_DPLP_HPD_PWEUP_INIT_MASK);
		phytium_dplp_write_reg(priv, 0x01, group_offset, PHYTIUM_DPLP_HPD_STATE_RESET);
		phytium_dplp_write_reg(priv, 0x00, group_offset, PHYTIUM_DPLP_HPD_STATE_RESET);
}
void phytium_dplp_init(struct ftd330_drm_private *priv)
{
	int i = 0;
	uint32_t physical_port = 0;
	FTD330_LOG_TRACE;
	for (i = 0; i < priv->info.total_pipes; i++) {
		physical_port = phytium_display_virtual_to_physical(priv->info.pipe_mask, i);
		phytium_dplp_init_port(priv->phytium_dp[physical_port]);
	}
}

void phytium_dplp_deinit_port(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  NULL;
	struct ftd330_drm_private *priv = NULL;
	uint32_t group_offset = 0;
	FTD330_LOG_TRACE;
	if (!phytium_dp) {
		pr_err("phytium_dp not init,skip dplp_deinit\n");
	}
	dev =  phytium_dp->dev;
	priv = dev->dev_private;
	group_offset = priv->dplp_reg_base[phytium_dp->port];
	phytium_dplp_write_reg(priv, 0x07, group_offset, PHYTIUM_DPLP_HPD_PWEUP_INIT_MASK);
}
void phytium_dplp_deinit(struct ftd330_drm_private *priv)
{
	int i = 0;
	uint32_t physical_port = 0;
	FTD330_LOG_TRACE;
	for (i = 0; i < priv->info.total_pipes; i++) {
		physical_port = phytium_display_virtual_to_physical(priv->info.pipe_mask, i);
		phytium_dplp_deinit_port(priv->phytium_dp[physical_port]);
	}
}

static void init_phy_parameters(struct phytium_dp_device *phytium_dp) {
	int port = phytium_dp->port;

	switch(port) {
		case 0:
		memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
				dp_drv_phy_val.dp0_drv_mgnfs_val,
			sizeof(dp_drv_phy_val.dp0_drv_mgnfs_val));
		memcpy(phytium_dp->phy_matrix.dp_cpost_val,
				dp_drv_phy_val.dp0_drv_cpost_val,
			sizeof(dp_drv_phy_val.dp0_drv_cpost_val));
		break;
		case 1:
		memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
				dp_drv_phy_val.dp1_drv_mgnfs_val,
			sizeof(dp_drv_phy_val.dp1_drv_mgnfs_val));
		memcpy(phytium_dp->phy_matrix.dp_cpost_val,
				dp_drv_phy_val.dp1_drv_cpost_val,
			sizeof(dp_drv_phy_val.dp1_drv_cpost_val));
		break;
		case 2:
		memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
				dp_drv_phy_val.dp2_drv_mgnfs_val,
			sizeof(dp_drv_phy_val.dp2_drv_mgnfs_val));
		memcpy(phytium_dp->phy_matrix.dp_cpost_val,
				dp_drv_phy_val.dp2_drv_cpost_val,
			sizeof(dp_drv_phy_val.dp2_drv_cpost_val));
		break;
		default:
			break;
		}
}

static void replace_custom_parameters(struct phytium_dp_device *phytium_dp) {
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	struct bios_table_info *bios_info = &priv->info.bios_info;

	switch(port) {
		case 0:
		if (bios_info->dp0_bios_mgnfs_valied) {
			memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
					bios_info->dp_bios_phy_val.dp0_bios_mgnfs_val,
					sizeof(bios_info->dp_bios_phy_val.dp0_bios_mgnfs_val));
			pr_info("dp-%d using acpi mgnfs parameter\n", port);
			}
		if (bios_info->dp0_bios_cpost_valied) {
			memcpy(phytium_dp->phy_matrix.dp_cpost_val,
					bios_info->dp_bios_phy_val.dp0_bios_cpost_val,
					sizeof(bios_info->dp_bios_phy_val.dp0_bios_cpost_val));
			pr_info("dp-%d using acpi cpost parameter\n", port);
			}
		if (bios_info->edp0_bios_bl_map_valied) {
			memcpy(phytium_dp->edp_bl_map.edp_bright_to_pwm,
					bios_info->edp_bios_bl_map.edp_bright_to_pwm[port],
					sizeof(bios_info->edp_bios_bl_map.edp_bright_to_pwm[port]));
			pr_info("dp-%d using acpi backlight map parameter\n", port);
		}
		break;
		case 1:
		if (bios_info->dp1_bios_mgnfs_valied) {
			memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
					bios_info->dp_bios_phy_val.dp1_bios_mgnfs_val,
					sizeof(bios_info->dp_bios_phy_val.dp1_bios_mgnfs_val));
			pr_info("dp-%d using acpi mgnfs parameter\n", port);
			}
		if (bios_info->dp1_bios_cpost_valied) {
			memcpy(phytium_dp->phy_matrix.dp_cpost_val,
					bios_info->dp_bios_phy_val.dp1_bios_cpost_val,
					sizeof(bios_info->dp_bios_phy_val.dp1_bios_cpost_val));
			pr_info("dp-%d using acpi cpost parameter\n", port);
			}
		if (bios_info->edp1_bios_bl_map_valied) {
			memcpy(phytium_dp->edp_bl_map.edp_bright_to_pwm,
					bios_info->edp_bios_bl_map.edp_bright_to_pwm[port],
					sizeof(bios_info->edp_bios_bl_map.edp_bright_to_pwm[port]));
			pr_info("dp-%d using acpi backlight map parameter\n", port);
		}
		break;
		case 2:
		if (bios_info->dp2_bios_mgnfs_valied) {
			memcpy(phytium_dp->phy_matrix.dp_mgnfs_val,
					bios_info->dp_bios_phy_val.dp2_bios_mgnfs_val,
					sizeof(bios_info->dp_bios_phy_val.dp2_bios_mgnfs_val));
			pr_info("dp-%d using acpi mgnfs parameter\n", port);
			}
		if (bios_info->dp2_bios_cpost_valied) {
			memcpy(phytium_dp->phy_matrix.dp_cpost_val,
					bios_info->dp_bios_phy_val.dp2_bios_cpost_val,
					sizeof(bios_info->dp_bios_phy_val.dp2_bios_cpost_val));
			pr_info("dp-%d using acpi cpost parameter\n", port);
			}
		if (bios_info->edp2_bios_bl_map_valied) {
			memcpy(phytium_dp->edp_bl_map.edp_bright_to_pwm,
					bios_info->edp_bios_bl_map.edp_bright_to_pwm[port],
					sizeof(bios_info->edp_bios_bl_map.edp_bright_to_pwm[port]));
			pr_info("dp-%d using acpi backlight map parameter\n", port);
		}
		break;
		default:
			break;
	}
}

int phytium_dp_init(struct drm_device *dev, int port)
{
	struct phytium_dp_device *phytium_dp = NULL;
#if defined(CONFIG_PHYTIUM_EDP_BL) || defined(CONFIG_PHYTIUM_LANE_TRAIN)
	struct ftd330_drm_private *priv = dev->dev_private;
#endif
	int ret = 0;
	int type;

	pr_info("FTD330 %s: port %d\n", __func__, port);
	phytium_dp = kzalloc(sizeof(*phytium_dp), GFP_KERNEL);
	if (!phytium_dp) {
		ret = -ENOMEM;
		goto failed_malloc_dp;
	}

	phytium_dp->dev = dev;
	phytium_dp->port = port;

	ftd330_dp_func_register(phytium_dp);
	phytium_dp_aux_init(phytium_dp);

	mutex_init(&phytium_dp->low_power_mutex);
	INIT_LIST_HEAD(&phytium_dp->phytium_mode.list);
#ifdef CONFIG_PHYTIUM_EDP_BL
	if (phytium_dp_is_edp(phytium_dp, phytium_port_virtual_to_physical(phytium_dp))) {
		phytium_dp->is_edp = true;
		type = DRM_MODE_CONNECTOR_eDP;

		if (ret) {
			DRM_ERROR("failed to initialize dp %d\n", phytium_dp->port);
				goto failed_init_dp;
			}

	} else {
		phytium_dp->is_edp = false;
		type = DRM_MODE_CONNECTOR_DisplayPort;
	}
#endif
	if (phytium_dp->is_edp) {
		ret = drm_encoder_init(dev, &phytium_dp->encoder, &phytium_encoder_funcs,
                                       DRM_MODE_ENCODER_TMDS, "DP %d", port);
	} else {
		ret = drm_encoder_init(dev, &phytium_dp->encoder, &phytium_encoder_funcs,
				       DRM_MODE_ENCODER_DPMST, "DP %d", port);
	}
	if (ret) {
		DRM_ERROR("failed to initialize encoder with drm\n");
		goto failed_encoder_init;
	}

	drm_encoder_helper_add(&phytium_dp->encoder, &phytium_encoder_helper_funcs);
	phytium_dp->encoder.possible_crtcs = BIT(port);

	phytium_dp->connector.dpms   = DRM_MODE_DPMS_OFF;
	ret = drm_connector_init(dev, &phytium_dp->connector, &phytium_connector_funcs, type);

	if (ret) {
		DRM_ERROR("failed to initialize connector with drm\n");
		goto failed_connector_init;
	}
	drm_connector_helper_add(&phytium_dp->connector, &phytium_connector_helper_funcs);
	drm_connector_attach_encoder(&phytium_dp->connector, &phytium_dp->encoder);

	phytium_dp->max_link_rate = phytium_dp->link_rate =
					drm_dp_bw_code_to_link_rate(DP_LINK_BW_8_1);
	if (phytium_dp->port == 0)
		phytium_dp->max_link_lane_count =
				phytium_dp->link_lane_count = source0_max_lane_count;
	else if (phytium_dp->port == 1)
		phytium_dp->max_link_lane_count =
				phytium_dp->link_lane_count = source1_max_lane_count;
	else if (phytium_dp->port == 2)
		phytium_dp->max_link_lane_count =
				phytium_dp->link_lane_count = source2_max_lane_count;
	else
		phytium_dp->max_link_lane_count = phytium_dp->link_lane_count = 4;

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	phytium_dp->phy_mode = priv->info.phy_mode[phytium_port_virtual_to_physical(phytium_dp)];
	if (phytium_dp->phy_mode == FTD330_PHY_MODE_DPX4) {
		phytium_dp->source_max_lane_count = phytium_dp->max_link_lane_count = phytium_dp->link_lane_count = 4;
	} else if (phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE01 
				|| phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE23) {
		phytium_dp->source_max_lane_count = phytium_dp->max_link_lane_count = phytium_dp->link_lane_count = 2;
	} else {
		DRM_ERROR("failed to initialize dp %d,phytium_dp->phy_mode error\n", phytium_dp->port);
		goto failed_init_dp;
	}
#endif
	/* Initialize customization parameters */
	init_phy_parameters(phytium_dp);
	memcpy(phytium_dp->edp_bl_map.edp_bright_to_pwm,
			edp_drv_bl_map.edp_bright_to_pwm,
			sizeof(edp_drv_bl_map.edp_bright_to_pwm));
	memcpy(phytium_dp->edp_bl_map.edp_pwm_to_bright,
			edp_drv_bl_map.edp_pwm_to_bright,
			sizeof(edp_drv_bl_map.edp_pwm_to_bright));

	replace_custom_parameters(phytium_dp);

	if (phytium_dp->is_edp) {
		phytium_dp_panel_init_panel_power_funcs(phytium_dp);
		/*
		 * Prevents abnormal display after just entering the OS
		*/
		phytium_edp_backlight_off(phytium_dp);
		phytium_dp_hw_init(phytium_dp);
		phytium_edp_panel_poweron(phytium_dp);
		if (priv->info.pwm_periodns && priv->info.pwm_clk_rate &&
								priv->info.pwm_div) {
			ret = phytium_pwm_set_parameter(priv, phytium_dp);
			if (ret)
				goto failed_init_dp;
			} else {
				DRM_ERROR("Missing edp pwm parameters\n");
				goto failed_init_dp;
			}
		phytium_dp->edp_edid = drm_get_edid(&phytium_dp->connector, &phytium_dp->aux.ddc);
		if (!phytium_dp->edp_edid)
			DRM_ERROR("get edp edid failed\n");
		phytium_dp_detect_dpcd(phytium_dp);
		phytium_dp_set_bios_common_rates_lanes(phytium_dp, phytium_dp->edp_edid);
		ret = drm_dp_dpcd_read(&phytium_dp->aux, DP_EDP_DPCD_REV,
					phytium_dp->edp_dpcd, sizeof(phytium_dp->edp_dpcd));
		if (ret < 0)
			DRM_ERROR("port %d get edp DPCD capability fail\n", phytium_dp->port);
		phytium_edp_dpcd_backlight_init(phytium_dp);
		phytium_dp_panel_init_backlight_funcs(phytium_dp);
#ifdef CONFIG_PHYTIUM_LOW_FPS
		phytium_dp->vrr.sink_support = phytium_sink_supports_vrr(phytium_dp);
		INIT_WORK(&phytium_dp->low_fps_work, phytium_dp_low_fps_work_func);
		phytium_dp->vrr.lowfps_work_init = true;
#endif
#ifdef CONFIG_PHYTIUM_PSR
		phytium_psr_init_dpcd(phytium_dp);
		INIT_WORK(&phytium_dp->psr_work, phytium_dp_psr_work_func);
		phytium_dp->psr.psr_work_init = true;
#endif
	} else {
		phytium_dp_hw_init(phytium_dp);
	}

	ret = phytium_dp_audio_codec_init(phytium_dp);
	if (ret) {
		DRM_ERROR("failed to initialize audio codec\n");
		goto failed_connector_init;
	}

	INIT_WORK(&phytium_dp->power_work, phytium_dp_power_work_func);

#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	phytium_dp->train_retry_count = 0;
	INIT_WORK(&phytium_dp->train_retry_work, phytium_dp_train_retry_work_fn);
#endif

	drm_connector_register(&phytium_dp->connector);

	priv->phytium_dp[phytium_display_virtual_to_physical(priv->info.pipe_mask, phytium_dp->port)] = phytium_dp;
	priv->need_power_down[phytium_display_virtual_to_physical(priv->info.pipe_mask, phytium_dp->port)] = false;
	return 0;

#if defined(CONFIG_PHYTIUM_LANE_TRAIN) || defined(CONFIG_PHYTIUM_EDP_BL)
failed_init_dp:
#endif
failed_connector_init:
failed_encoder_init:
	if (phytium_dp)
		kfree(phytium_dp);
failed_malloc_dp:
	return ret;
}



void phytium_dp_platform_init(struct drm_device *dev)
{
	int i = 0, j = 0;
	struct ftd330_drm_private *priv = dev->dev_private;
	int ret = 0;
	int display_index = 0;

	mutex_init(&priv->power_mutex);
	spin_lock_init(&priv->hotplug_irq_lock);
	
	for (i = DISPLAY_0; i < DISPLAY_NUM; i++) {
		if (priv->info.pipe_mask & BIT(i)) {
			priv->dp_reg_base[display_index] = PHYTIUM_FTD330_DP_REG_OFFSET +
							i*PHYTIUM_FTD330_DP_REG_INTERVAL;
			priv->dplp_reg_base[display_index] = PHYTIUM_FTD330_DPLP_REG_OFFSET +
							i*PHYTIUM_FTD330_DPLP_REG_INTERVAL;
			priv->phy_access_base[display_index] = PHYTIUM_FTD330_DP_PHY_REG_OFFSET +
							i*PHYTIUM_FTD330_DP_PHY_REG_INTERVAL;
			display_index++;
		}
	}

	if (priv->info.edp_mask)
		priv->edp_pwm_base = PHYTIUM_FTD330_EDP_PWM_REG_OFFSET + 0x400;

	for_each_pipe(priv, j) {
		ret = phytium_dp_init(dev, j);
		if (ret) {
			pr_info("phytium_dp_init(pipe %d) return failed\n", i);
			return;
		}
	}
}

