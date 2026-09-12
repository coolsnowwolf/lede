# SPDX-License-Identifier: GPL-2.0
ccflags-y := -I $(src)
ifeq ($(CONFIG_PHYTIUM_CHIP_9x00), y)
ccflags-y += -I $(src)/9x00
else ifeq ($(CONFIG_PHYTIUM_CHIP_FTD330), y)
ccflags-y += -I $(src)/FTD330
else
ccflags-y += -I $(src)/FTD330
endif
#ccflags-y += -Wall  -Wextra

ftd330-drm-dc-objs := ftd330_crtc.o \
		ftd330_drv.o \
		ftd330_fb.o \
		ftd330_gem.o \
		ftd330_plane.o \
		ftd330_dc_lut.o \
		ftd330_dc_scale.o \
		ftd330_dc_property.o \
		ftd330_dc_drm_property.o \
		ftd330_simple_enc.o \
		phytium_dp.o \
		ftd330_dp.o \
		phytium_se_communicate.o \
		phytium_panel.o \
		phytium_edp_pwm.o \
		phytium_psr.o \
		phytium_vrr.o \
		phytium_dp_debugfs.o \
		phytium_parse_bios.o



# link to the FTD330 specify info
ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x31b), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x31b.o
else ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x311), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x311.o
else ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x310), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x310.o
else ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x30b), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x30b.o
else ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x331), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x331.o
else ifeq ($(CONFIG_PHYTIUM_DCFTD330_0x335), y)
ftd330-drm-dc-objs += FTD330/info/ftd330_dc_info_FTD330_0x335.o
else
ftd330-drm-dc-objs += FTD330/info/phytium_dc_info.o
endif
# FTD330 other objs
ftd330-drm-dc-objs += FTD330/ftd330_dc_hw.o
ftd330-drm-dc-objs += FTD330/ftd330_dc.o
ftd330-drm-dc-objs += FTD330/preprocess/ftd330_dc_plane_blender.o
ftd330-drm-dc-objs += FTD330/preprocess/ftd330_dc_preprocess.o
ftd330-drm-dc-objs += FTD330/postprocess/ftd330_dc_postprocess.o

ftd330-drm-dc-$(CONFIG_PHYTIUM_WRITEBACK) += ftd330_writeback.o
ftd330-drm-dc-$(CONFIG_PHYTIUM_WRITEBACK) += FTD330/writeback/ftd330_dc_writeback.o
ftd330-drm-dc-$(CONFIG_PHYTIUM_DEBUG) += ftd330_debug.o
ftd330-drm-dc-$(CONFIG_PHYTIUM_MMU) += ftd330_dc_mmu.o
ftd330-drm-dc-$(CONFIG_PHYTIUM_DEC) += FTD330/ftd330_dc_dec.o
ftd330-drm-dc-$(CONFIG_PHYTIUM_FBDEV_ON)   += phytium_fbdev.o

# phytium internal
ftd330-drm-dc-$(CONFIG_PHYTIUM_VIRTUAL_DISPLAY) += ftd330_virtual.o

obj-$(CONFIG_DRM_PHYTIUM) += ftd330-drm-dc.o

CFLAGS_REMOVE_ftd330_dc_scale.o += -mgeneral-regs-only
