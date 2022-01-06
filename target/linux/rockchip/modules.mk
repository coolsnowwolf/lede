# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 OpenWrt.org

define KernelPackage/drm-rockchip
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Rockchip DRM support
  DEPENDS:=@TARGET_rockchip +kmod-backlight +kmod-drm-kms-helper +kmod-multimedia-input
  KCONFIG:= \
	CONFIG_DRM_ROCKCHIP \
	CONFIG_DRM_LOAD_EDID_FIRMWARE=y \
	CONFIG_DRM_FBDEV_EMULATION=y \
	CONFIG_DRM_FBDEV_OVERALLOC=100 \
	CONFIG_DRM_BRIDGE \
	CONFIG_HDMI \
	CONFIG_PHY_ROCKCHIP_INNO_HDMI \
	CONFIG_DRM_DW_HDMI \
	CONFIG_DRM_DW_HDMI_CEC \
	CONFIG_ROCKCHIP_ANALOGIX_DP=n \
	CONFIG_ROCKCHIP_CDN_DP=n \
	CONFIG_ROCKCHIP_DW_HDMI=y \
	CONFIG_ROCKCHIP_INNO_HDMI=y \
	CONFIG_ROCKCHIP_DW_MIPI_DSI=y \
	CONFIG_ROCKCHIP_LVDS=y \
	CONFIG_ROCKCHIP_RGB=n \
	CONFIG_ROCKCHIP_RK3066_HDMI=n \
	CONFIG_DRM_PANEL \
	CONFIG_DRM_PANEL_BRIDGE \
	CONFIG_DRM_PANEL_SIMPLE
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi-cec.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-mipi-dsi.ko \
	$(LINUX_DIR)/drivers/media/cec/cec.ko@lt5.10 \
	$(LINUX_DIR)/drivers/media/cec/core/cec.ko@ge5.10 \
	$(LINUX_DIR)/drivers/phy/rockchip/phy-rockchip-inno-hdmi.ko \
	$(LINUX_DIR)/drivers/gpu/drm/panel/panel-simple.ko \
	$(LINUX_DIR)/drivers/gpu/drm/rockchip/rockchipdrm.ko
  AUTOLOAD:=$(call AutoProbe,rockchipdrm phy-rockchip-inno-hdmi dw-hdmi-cec)
endef

define KernelPackage/drm-rockchip/description
  Direct Rendering Manager (DRM) support for Rockchip
endef

$(eval $(call KernelPackage,drm-rockchip))

define KernelPackage/gpu-lima
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Mali-4xx GPU support
  DEPENDS:=@TARGET_rockchip +kmod-drm
  KCONFIG:= \
	CONFIG_DRM_VGEM \
	CONFIG_DRM_GEM_CMA_HELPER=y \
	CONFIG_DRM_LIMA
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/vgem/vgem.ko \
	$(LINUX_DIR)/drivers/gpu/drm/scheduler/gpu-sched.ko \
	$(LINUX_DIR)/drivers/gpu/drm/lima/lima.ko
  AUTOLOAD:=$(call AutoProbe,lima vgem)
endef

define KernelPackage/gpu-lima/description
  Open-source reverse-engineered driver for Mali-4xx GPUs
endef

$(eval $(call KernelPackage,gpu-lima))
