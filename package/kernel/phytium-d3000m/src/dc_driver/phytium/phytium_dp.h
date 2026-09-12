/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_DP_H__
#define __PHYTIUM_DP_H__

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_encoder.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_modes.h>
#include <sound/hdmi-codec.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
#include <drm/display/drm_dp.h>
#include <drm/display/drm_dp_helper.h>
#else
#include <drm/drm_dp_helper.h>
#endif
#include "ftd330_drv.h"
struct phytium_dp_device;

#include "phytium_panel.h"
#include "phytium_edp_pwm.h"

#define DEBUG_LOG 0

#define FTD330_LOG_TRACE \
       if (ftd330_phytium_dc_log_level > FTD330_PHYTIUM_DC_NO_LOG) {\
		pr_err("%s %d\n",__func__,__LINE__); \
       }
#define PHYTIUM_FTD330_DP_REG_OFFSET 0x16000
#define PHYTIUM_FTD330_DP_REG_INTERVAL 0x1000

#define PHYTIUM_FTD330_DPLP_REG_OFFSET 0x1C000
#define PHYTIUM_FTD330_DPLP_REG_INTERVAL 0x400

#define PHYTIUM_FTD330_DP_PHY_REG_OFFSET 0x19000
#define PHYTIUM_FTD330_DP_PHY_REG_INTERVAL 0x1000

#define PHYTIUM_FTD330_EDP_PWM_REG_OFFSET 0x1D000

#define LOWFPS_CNT_NUMS 0x258
#define PSR_CNT_NUMS 0x4B0

#ifndef DP_MAX_DOWNSTREAM_PORTS
#define DP_MAX_DOWNSTREAM_PORTS                    0x10
#endif

#ifndef DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT
#define DP_EXTENDED_RECEIVER_CAP_FIELD_PRESENT		(1 << 7)
#endif

struct audio_info {
	int sample_rate;
	int channels;
	int sample_width;
};

struct dp_audio_n_m {
	int sample_rate;
	int link_rate;
	u16 m;
	u16 n;
};

struct phytium_dp_compliance {
	unsigned long test_type;
	uint32_t test_link_rate;
	u8 test_lane_count;
	bool test_active;
	u8 reserve[2];
};

struct phytium_dp_func {
	int (*dp_hw_reset)(struct phytium_dp_device *phytium_dp);
	bool (*dp_hw_spread_is_enable)(struct phytium_dp_device *phytium_dp);
	int (*dp_hw_set_backlight)(struct phytium_dp_device *phytium_dp, uint32_t level);
	uint32_t (*dp_hw_get_backlight)(struct phytium_dp_device *phytium_dp);
	void (*dp_hw_disable_backlight)(struct phytium_dp_device *phytium_dp);
	void (*dp_hw_enable_backlight)(struct phytium_dp_device *phytium_dp);
	void (*dp_hw_poweroff_panel)(struct phytium_dp_device *phytium_dp);
	void (*dp_hw_poweron_panel)(struct phytium_dp_device *phytium_dp);
	int (*dp_hw_init_phy)(struct phytium_dp_device *phytium_dp);
	void (*dp_hw_set_phy_lane_setting)(struct phytium_dp_device *phytium_dp,
					   uint32_t link_rate, uint8_t train_set);
	int (*dp_hw_set_phy_lane_and_rate)(struct phytium_dp_device *phytium_dp,
					   uint8_t link_lane_count,
					   uint32_t link_rate);
	void (*dp_hw_config_phy_power)(struct phytium_dp_device *phytium_dp,
						bool enable);
};

struct phytium_dp_hpd_state {
	bool hpd_event_state;
	bool hpd_irq_state;
	bool hpd_raw_state;
	bool hpd_irq_enable;
};

struct phytium_display_mode {
	struct list_head list;
	u32 clock;
	u16 h_active;
	u16 h_total;
	u16 h_sync_start;
	u16 h_sync_end;
	u16 v_active;
	u16 v_total;
	u16 v_sync_start;
	u16 v_sync_end;
	u32 flags;
	u8 freq;
	u8 type;
	u8 bpc;
	bool reduced_bpc;
};

struct phytium_dplp_frame_compare_state {
	bool psr_available;
	bool frame_change_in_psr;
	bool psr_exit;
	bool lowfps_available;
	bool lowfps_exit;
};

struct phytium_psr {
	bool sink_support;
	bool sink_psr2_support;
	bool psr2_enabled;
	bool enabled;
	bool active;
	bool link_standby;
	bool colorimetry_support;
	bool sink_need_train_on_exit;
	bool psr_work_init;
	bool sink_not_reliable;
	int psr_setup_time_us;
	unsigned long last_jiffies;
	uint8_t sink_sync_latency;
	uint16_t frame_change_irq_nums;
	uint16_t su_w_granularity;
	uint16_t su_y_granularity;
	uint8_t sink_setup_time;
};

struct phytium_vrr {
	bool enabled;
	bool sink_support;
	bool lowfps_work_init;
	struct detailed_data_monitor_range *range;
	struct dc_hw_display_mode *vrr_mode;
};


struct phytium_dp_device {
	struct drm_device *dev;
	struct drm_encoder encoder;
	struct drm_connector connector;
	int port;
	struct drm_display_mode mode;
	struct phytium_display_mode phytium_mode;
	bool	link_trained;
	bool	detect_done;
	bool	is_edp;
	bool	reserve0;
	struct drm_dp_aux aux;
	unsigned char dpcd[DP_RECEIVER_CAP_SIZE];
	uint8_t edp_dpcd[EDP_DISPLAY_CTL_CAP_SIZE];
	uint8_t psr_dpcd[EDP_PSR_RECEIVER_CAP_SIZE];
	unsigned char downstream_ports[DP_MAX_DOWNSTREAM_PORTS];
	unsigned char sink_count;
	int freq;
	unsigned int bpc;

	int *source_rates;
	int num_source_rates;
	int sink_rates[DP_MAX_SUPPORTED_RATES];
	int num_sink_rates;
	int common_rates[DP_MAX_SUPPORTED_RATES];
	int num_common_rates;

	int source_max_lane_count;
	int sink_max_lane_count;
	int common_max_lane_count;

	int max_link_rate;
	int max_link_lane_count;
	int max_pass_link_rate;
	int max_pass_lane_count;
	int link_rate;
	int link_lane_count;
	struct work_struct train_retry_work;
	int train_retry_count;
	uint32_t trigger_train_fail;
	unsigned char phy_mode;

	unsigned char train_set[4];
	bool has_audio;
	bool fast_train_support;
	bool hw_spread_enable;
	bool reserve[1];
	struct platform_device *audio_pdev;
	struct audio_info audio_info;
	hdmi_codec_plugged_cb plugged_cb;
	struct device *codec_dev;
	struct phytium_dp_compliance compliance;
	struct phytium_dp_func *funcs;
	struct phytium_dp_hpd_state dp_hpd_state;
	struct phytium_dplp_frame_compare_state dplp_frame_compare_state;

	struct phytium_panel panel;
	struct drm_display_mode native_mode;

	struct work_struct power_work;
	struct work_struct psr_work;
	struct work_struct low_fps_work;

	struct phytium_psr psr;
	struct phytium_vrr vrr;
	struct mutex low_power_mutex;
	struct edid *edp_edid;

	struct dp_phy_matrix phy_matrix;
	struct edp_backlight_map edp_bl_map;
};
enum phytium_dp_power_operation {
	PHYTIUM_POWER_KEEP,
	FROM_ON_TO_OFF,
	FROM_OFF_TO_ON,
};

union phytium_phy_tp {
	struct {
		/* DpcdPhyTestPatterns. This field is 2 bits for DP1.1
		 * and 3 bits for DP1.2.
		 */
		uint8_t PATTERN	:3;
		uint8_t RESERVED	:5;
	} bits;
	uint8_t raw;
};

static inline unsigned int
phytium_readl_reg(struct ftd330_drm_private *priv, uint32_t group_offset, uint32_t reg_offset)
{
	unsigned int data;

	data = readl(priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Read 32'h%08x 32'h%08x\n", group_offset + reg_offset, data);
#endif
	return data;
}

static inline void
phytium_writel_reg(struct ftd330_drm_private *priv, uint32_t data,
			    uint32_t group_offset, uint32_t reg_offset)
{

	writel(data, priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Write 32'h%08x 32'h%08x\n", group_offset + reg_offset, data);
#endif
}

static inline void
phytium_writeb_reg(struct ftd330_drm_private *priv, uint8_t data,
			   uint32_t group_offset, uint32_t reg_offset)
{
	writeb(data, priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Write 32'h%08x 8'h%08x\n", group_offset + reg_offset, data);
#endif
}

static inline void
phytium_dplp_write_reg(struct ftd330_drm_private *priv, uint32_t data,
						   uint32_t group_offset, uint32_t reg_offset)
{
	   writel(data, priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	   pr_info("dplp Write 32'h%08x 32'h%08x\n", group_offset+reg_offset, data);
#endif
}
static inline uint32_t
phytium_dplp_read_reg(struct ftd330_drm_private *priv, uint32_t group_offset, uint32_t reg_offset)
	{
		uint32_t data = 0;

		data = readl(priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
		pr_info("dplp read  32'h%08x 32'h%08x\n", group_offset+reg_offset, data);
#endif
		return data;
}

#define for_each_pipe(__dev_priv, __p) \
	for ((__p) = 0; (__p) < __dev_priv->info.total_pipes; (__p)++)

#define phytium_edid_encode_panel_id(vend_chr_0, vend_chr_1, vend_chr_2, product_id) \
	((((u32)(vend_chr_0) - '@') & 0x1f) << 26 | \
	 (((u32)(vend_chr_1) - '@') & 0x1f) << 21 | \
	 (((u32)(vend_chr_2) - '@') & 0x1f) << 16 | \
	 ((product_id) & 0xffff))

/* PHY test patterns
 * The order of test patterns follows DPCD register PHY_TEST_PATTERN (0x248)
 */
enum phytium_dpcd_phy_tp {
	PHYTIUM_PHY_TP_NONE = 0,
	PHYTIUM_PHY_TP_D10_2,
	PHYTIUM_PHY_TP_SYMBOL_ERROR,
	PHYTIUM_PHY_TP_PRBS7,
	PHYTIUM_PHY_TP_80BIT_CUSTOM,
	PHYTIUM_PHY_TP_CP2520_1,
	PHYTIUM_PHY_TP_CP2520_2,
	PHYTIUM_PHY_TP_CP2520_3,
};
#define encoder_to_dp_device(x) container_of(x, struct phytium_dp_device, encoder)
#define connector_to_dp_device(x) container_of(x, struct phytium_dp_device, connector)
#define panel_to_dp_device(x) container_of(x, struct phytium_dp_device, panel)
#define train_retry_to_dp_device(x)	container_of(x, struct phytium_dp_device, train_retry_work)
void phytium_phy_writel(struct phytium_dp_device *phytium_dp, uint32_t address, uint32_t data);
uint32_t phytium_phy_readl(struct phytium_dp_device *phytium_dp, uint32_t address);
bool is_dp_powered(struct phytium_dp_device *phytium_dp);
#define PHYTIUM_DP_AUDIO_ID	(('P' << 24) + ('H' << 16) + ('Y' << 8))
int phytium_dp_init(struct drm_device *dev, int pipe);
void phytium_edp_power_on(struct phytium_dp_device *phytium_dp);
void phytium_edp_power_off(struct phytium_dp_device *phytium_dp);
int phytium_dp_resume(struct drm_device *drm_dev);
int phytium_dp_suspend(struct drm_device *drm_dev);
void phytium_dp_hpd_irq_setup(struct drm_device *dev, bool enable, bool handle_irq);
irqreturn_t phytium_dp_hpd_irq_handler(int irq, void *data);
irqreturn_t phytium_dp_power_on_irq_handler(int irq, void *data);
void phytium_dp_hpd_work_func(struct work_struct *work);
void phytium_dp_power_work_func(struct work_struct *work);
const struct dp_audio_n_m *phytium_dp_audio_get_n_m(int link_rate, int sample_rate);
void phytium_dplp_init(struct ftd330_drm_private *priv);
void phytium_dplp_init_port(struct phytium_dp_device *phytium_dp);
void phytium_dplp_deinit(struct ftd330_drm_private *priv);
void phytium_dplp_deinit_port(struct phytium_dp_device *phytium_dp);
void phytium_dp_hw_hpd_irq_setup(struct phytium_dp_device *phytium_dp, bool enable, bool handle_irq);
enum drm_mode_status
phytium_encoder_mode_valid(struct drm_encoder *encoder,
						const struct drm_display_mode
						*mode);
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
void phytium_display_power_request_on(struct drm_device *dev, int display_id, bool handle_irq);
void phytium_display_power_request_off(struct drm_device *dev, int display_id);
#endif
void phytium_dp_platform_init(struct drm_device *dev);
int phytium_dp_start_link_train(struct phytium_dp_device *phytium_dp);
bool phytium_dp_fast_link_train(struct phytium_dp_device *phytium_dp);
void phytium_dp_fast_link_train_detect(struct phytium_dp_device *phytium_dp);
void phytium_dp_hw_enable_audio(struct phytium_dp_device *phytium_dp);
void phytium_power_off_extra_display(struct ftd330_drm_private *priv);

extern int source0_max_lane_count;
extern int source1_max_lane_count;
extern int source2_max_lane_count;
extern int dc_fast_training_check;
extern int link_dynamic_adjust;
extern int dc_fake_mode_enable;
extern int num_source_rates;
#endif /* __PHYTIUM_DP_H__ */
