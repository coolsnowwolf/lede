/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_DISPLAY_DRV_H__
#define __PHYTIUM_DISPLAY_DRV_H__

#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
#include <drm/drmP.h>
#endif
#include <drm/drm_print.h>
#include <drm/drm_fb_helper.h>

#define DEBUG_LOG 0

#define PHYTIUM_FORMAT_MAX_PLANE	3
#define DP_MAX_DOWNSTREAM_PORTS		0x10

#define DRV_NAME	"dc"
#define DRV_DESC	"phytium dc"
#define DRV_DATE	"20201220"
#define DRV_MAJOR	1
#define DRV_MINOR	1

/* come from GPU */
#define	DRM_FORMAT_MOD_VENDOR_PHYTIUM	0x92

/* dc:mode0 8x8 16bpp  gpu: FBCDC_8X8_V10 */
#define	DRM_FORMAT_MOD_PHYTIUM_TILE_MODE0_FBCDC	fourcc_mod_code(PHYTIUM, 21)
/* dc:mode3 8x4 32bpp  gpu: FBCDC_16X4_v10 */
#define	DRM_FORMAT_MOD_PHYTIUM_TILE_MODE3_FBCDC	fourcc_mod_code(PHYTIUM, 22)

#define	PIPE_MASK_SHIFT			0x0
#define	PIPE_MASK_MASK			0x7
#define	EDP_MASK_SHIFT			0x3
#define	EDP_MASK_MASK			0x7

enum phytium_platform {
	PHYTIUM_PLATFORM_UNINITIALIZED = 0,
	PHYTIUM_PLATFORM_PX210,
	PHYTIUM_PLATFORM_PE220X,
};

enum phytium_mem_state_type {
	PHYTIUM_MEM_VRAM_TOTAL = 0,
	PHYTIUM_MEM_VRAM_ALLOC,
	PHYTIUM_MEM_SYSTEM_CARVEOUT_TOTAL,
	PHYTIUM_MEM_SYSTEM_CARVEOUT_ALLOC,
	PHYTIUM_MEM_SYSTEM_UNIFIED_ALLOC,
	PHYTIUM_MEM_STATE_TYPE_COUNT,
};

#define MEMORY_TYPE_VRAM		0x1
#define MEMORY_TYPE_SYSTEM_CARVEOUT	0x2
#define MEMORY_TYPE_SYSTEM_UNIFIED	0x4

#define IS_PLATFORM(priv, p) ((priv)->info.platform_mask & BIT(p))

#define IS_PX210(priv)		IS_PLATFORM(priv, PHYTIUM_PLATFORM_PX210)
#define IS_PE220X(priv)		IS_PLATFORM(priv, PHYTIUM_PLATFORM_PE220X)

struct phytium_device_info {
	unsigned char platform_mask;
	unsigned char pipe_mask;
	unsigned char num_pipes;
	unsigned char total_pipes;
	unsigned char edp_mask;
	unsigned int crtc_clock_max;
	unsigned int hdisplay_max;
	unsigned int vdisplay_max;
	unsigned int backlight_max;
	unsigned long address_mask;
};

struct phytium_display_private {
	/* hw */
	void __iomem *regs;
	void __iomem *vram_addr;
	struct phytium_device_info info;
	char support_memory_type;
	char reserve[3];
	uint32_t dc_reg_base[3];
	uint32_t dcreq_reg_base[3];
	uint32_t dp_reg_base[3];
	uint32_t address_transform_base;
	uint32_t phy_access_base[3];

	/* drm */
	struct drm_device *dev;
	int irq;

	/* fb_dev */
	struct drm_fb_helper fbdev_helper;
	struct phytium_gem_object *fbdev_phytium_gem;

	int save_reg[3];
	struct list_head gem_list_head;

	struct work_struct hotplug_work;
	spinlock_t hotplug_irq_lock;

	void (*vram_hw_init)(struct phytium_display_private *priv);
	void (*display_shutdown)(struct drm_device *dev);
	int (*display_pm_suspend)(struct drm_device *dev);
	int (*display_pm_resume)(struct drm_device *dev);
	void (*dc_hw_clear_msi_irq)(struct phytium_display_private *priv, uint32_t phys_pipe);
	int (*dc_hw_fb_format_check)(const struct drm_mode_fb_cmd2 *mode_cmd, int count);

	struct gen_pool *memory_pool;
	resource_size_t pool_phys_addr;
	resource_size_t pool_size;
	void *pool_virt_addr;
	uint64_t mem_state[PHYTIUM_MEM_STATE_TYPE_COUNT];

	/* DMA info */
	int dma_inited;
	struct dma_chan *dma_chan;
};

static inline unsigned int
phytium_readl_reg(struct phytium_display_private *priv, uint32_t group_offset, uint32_t reg_offset)
{
	unsigned int data;

	data = readl(priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Read 32'h%08x 32'h%08x\n", group_offset + reg_offset, data);
#endif
	return data;
}

static inline void
phytium_writel_reg(struct phytium_display_private *priv, uint32_t data,
			    uint32_t group_offset, uint32_t reg_offset)
{

	writel(data, priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Write 32'h%08x 32'h%08x\n", group_offset + reg_offset, data);
#endif
}

static inline void
phytium_writeb_reg(struct phytium_display_private *priv, uint8_t data,
			   uint32_t group_offset, uint32_t reg_offset)
{
	writeb(data, priv->regs + group_offset + reg_offset);
#if DEBUG_LOG
	pr_info("Write 32'h%08x 8'h%08x\n", group_offset + reg_offset, data);
#endif
}

#define for_each_pipe(__dev_priv, __p) \
	for ((__p) = 0; (__p) < __dev_priv->info.total_pipes; (__p)++)

#define for_each_pipe_masked(__dev_priv, __p) \
	for ((__p) = 0; (__p) < __dev_priv->info.total_pipes; (__p)++) \
		for_each_if((__dev_priv->info.pipe_mask) & BIT(__p))

int phytium_get_virt_pipe(struct phytium_display_private *priv, int phys_pipe);
int phytium_get_phys_pipe(struct phytium_display_private *priv, int virt_pipe);
int phytium_wait_cmd_done(struct phytium_display_private *priv,
				    uint32_t register_offset,
				    uint32_t request_bit,
				    uint32_t reply_bit);
void phytium_display_private_init(struct phytium_display_private *priv, struct drm_device *dev);

extern struct drm_driver phytium_display_drm_driver;
extern int dc_fake_mode_enable;
extern int dc_fast_training_check;
extern int num_source_rates;
extern int source_max_lane_count;
extern int link_dynamic_adjust;

#endif /* __PHYTIUM_DISPLAY_DRV_H__ */
