/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DRV_H__
#define __FTD330_DRV_H__

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#include <drm/drm_gem.h>
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#endif


#include "ftd330_plane.h"
#ifdef CONFIG_PHYTIUM_MMU
#include "ftd330_dc_mmu.h"
#endif

#include <drm/drm_fb_helper.h>

extern int ftd330_phytium_dc_log_level;
#define FTD330_LOG(fmt, ...) \
	if (ftd330_phytium_dc_log_level > FTD330_PHYTIUM_DC_NO_LOG) { \
		pr_err(fmt, ##__VA_ARGS__);\
	}
#define DC_DRIVER_VERSION "1.1.2"

#ifndef DRM_FORMAT_MOD_VENDOR_PHYTIUM
#define DRM_FORMAT_MOD_VENDOR_PHYTIUM  0x06
#endif

#ifndef DRM_FTD330_FB_GET
#define DRM_FTD330_FB_GET 0x08
#endif

#ifndef DRM_IOCTL_FTD330_FB_GET
#define DRM_IOCTL_FTD330_FB_GET \
        DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_FB_GET, struct drm_phytium_display_fb_cmd2)
#endif


#ifndef DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED
#define DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED    fourcc_mod_code(PHYTIUM, 2)
#endif

#ifndef DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC
#define DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC fourcc_mod_code(PHYTIUM, 5)
#endif


enum ftd330_phytium_dc_log {
	FTD330_PHYTIUM_DC_NO_LOG,
	FTD330_PHYTIUM_DC_LOG_ON,
};

struct dp_phy_matrix {
	int dp_mgnfs_val[4][4][4];
	int dp_cpost_val[4][4][4];
};

struct edp_backlight_map {
	int edp_bright_to_pwm[101];
	int edp_pwm_to_bright[101];
};

struct dp_bios_phy_matrix {
	int dp0_bios_mgnfs_val[4][4][4];
	int dp1_bios_mgnfs_val[4][4][4];
	int dp2_bios_mgnfs_val[4][4][4];

	int dp0_bios_cpost_val[4][4][4];
	int dp1_bios_cpost_val[4][4][4];
	int dp2_bios_cpost_val[4][4][4];
};

struct edp_bios_backlight_map {
	int edp_bright_to_pwm[3][101];
	int edp_pwm_to_bright[3][101];
};

struct edp_bios_panel_timing {
	int panel_power_up_delay[3][1];
	int backlight_on_delay[3][1];
	int backlight_off_delay[3][1];
	int panel_power_down_delay[3][1];
	int panel_power_cycle_delay[3][1];
};

struct bios_panel_info {
	uint32_t panel_id;
	u8 max_lane_count;
	u8 max_link_rate;
	int num_link_rate;
	bool valid;
};

struct bios_table_info {
	void __iomem *para_table_base;
	struct dp_bios_phy_matrix dp_bios_phy_val;
	bool dp0_bios_mgnfs_valied;
	bool dp1_bios_mgnfs_valied;
	bool dp2_bios_mgnfs_valied;
	bool dp0_bios_cpost_valied;
	bool dp1_bios_cpost_valied;
	bool dp2_bios_cpost_valied;

	struct edp_bios_backlight_map edp_bios_bl_map;
	bool edp0_bios_bl_map_valied;
	bool edp1_bios_bl_map_valied;
	bool edp2_bios_bl_map_valied;

	struct edp_bios_panel_timing edp_bios_panel_time;
	bool edp0_bios_panel_time_valied;
	bool edp1_bios_panel_time_valied;
	bool edp2_bios_panel_time_valied;

	struct bios_panel_info *panels;
	u32 panel_count;
};

struct phytium_device_info {
	unsigned char total_pipes;
	unsigned char pipe_mask;
	unsigned char edp_mask;
	unsigned int water_mark[DISPLAY_NUM];
	unsigned int qos[DISPLAY_NUM];
	unsigned char overlay_enable;
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	unsigned int phy_mode[3];
#endif
	unsigned int crtc_clock_max;
	unsigned int hdisplay_max;
	unsigned int vdisplay_max;
	unsigned int backlight_max;
	unsigned int backlight_min;
	unsigned long address_mask;
	uint32_t pwm_periodns;
	uint32_t pwm_clk_rate;
	uint32_t pwm_div;
	uint32_t pwm_freq;
	uint32_t para_table_addr[2];
	struct bios_table_info bios_info;
	bool para_table_valid;
	bool edp_boot_need_poweron;
	bool edp_s3_need_poweron;
	bool edp_s4_need_poweron;
};


/*
 *
 * @dma_dev: device for DMA API.
 *  - use the first attached device if support iommu
 *    else use drm device (only contiguous buffer support)
 * @domain: iommu domain for DRM.
 *  - all DC IOMMU share same domain to reduce mapping
 * @pitch_alignment: buffer pitch alignment required by sub-devices.
 *
 */
struct ftd330_drm_private {
	struct device *dma_dev;
	/* when we have more than one display core, this need to be an array */
	struct device *dc_dev;

	struct iommu_domain *domain;
#ifdef CONFIG_PHYTIUM_MMU
	dc_mmu * mmu;
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
	struct file *dc_capture_fp;
#endif
	unsigned int pitch_alignment;
	unsigned int addr_alignment;

	u8 intr_dest;
	u32 intr_mask;

	struct phytium_device_info info;
	void __iomem *regs;
	void __iomem *se_regs;
	uint32_t dp_reg_base[DISPLAY_NUM];
	uint32_t dplp_reg_base[DISPLAY_NUM];
	uint32_t address_transform_base;
	uint32_t phy_access_base[DISPLAY_NUM];

#ifdef CONFIG_PHYTIUM_PCIE
	struct pci_dev *pdev;
#else
	struct platform_device *pdev;
#endif
	struct drm_device *drm_dev;

	struct gen_pool *mem_pool;
	unsigned long mem_pool_start_address_phy;
	void *mem_pool_start_address_virt;
	unsigned long  mem_pool_size;

	/* fb_dev */
	struct drm_fb_helper fbdev_helper;
	struct ftd330_gem_object *fbdev_phytium_gem;
	struct work_struct fbdev_init_work;
	struct work_struct hotplug_work;
	bool phytium_log_enable;
	bool low_power_enable[DISPLAY_NUM];
	bool need_power_down[DISPLAY_NUM];

	/*edp_pwm*/
	uint32_t edp_pwm_base;

	/*dp_hotplug_mutex*/
	struct mutex power_mutex;
	struct list_head gem_list_head;
	struct phytium_dp_device *phytium_dp[DISPLAY_NUM];
	bool power_status_save[DISPLAY_NUM];
	spinlock_t hotplug_irq_lock;
};

int ftd330_drm_iommu_attach_device(struct drm_device *drm_dev, struct device *dev);

void ftd330_drm_iommu_detach_device(struct drm_device *drm_dev, struct device *dev);

void ftd330_drm_update_alignment(struct drm_device *drm_dev, unsigned int pitch_align,
			     unsigned int addr_align);
int phytium_dc_registers_init(struct ftd330_drm_private *priv, u32 dc_id);

void phytium_update_timing_for_drm_display_mode(struct drm_display_mode *drm_modde,const struct drm_display_mode *native_mode);

static inline struct device *to_dma_dev(struct drm_device *dev)
{
	struct ftd330_drm_private *priv = dev->dev_private;

	return priv->dma_dev;
}

static inline bool is_iommu_enabled(struct drm_device *dev)
{
	struct ftd330_drm_private *priv = dev->dev_private;

	return priv->domain != NULL ? true : false;
}
extern struct platform_driver phyitum_dp_platform_driver;
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
extern void phytium_display_power_request_on(struct drm_device *dev, int display_id, bool handle_irq);
extern void phytium_display_power_request_off(struct drm_device *dev, int display_id);
#endif
#endif /* __FTD330_DRV_H__ */
