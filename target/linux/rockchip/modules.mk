# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 OpenWrt.org

define KernelPackage/drm-rockchip
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Rockchip DRM support
  DEPENDS:=@TARGET_rockchip +kmod-backlight +kmod-drm-kms-helper \
	+kmod-multimedia-input +LINUX_6_0:kmod-drm-display-helper
  KCONFIG:= \
	CONFIG_DRM_ROCKCHIP \
	CONFIG_DRM_LOAD_EDID_FIRMWARE=y \
	CONFIG_DRM_FBDEV_EMULATION=y \
	CONFIG_DRM_FBDEV_OVERALLOC=100 \
	CONFIG_DRM_BRIDGE=y \
	CONFIG_HDMI=y \
	CONFIG_PHY_ROCKCHIP_INNO_HDMI \
	CONFIG_DRM_DW_HDMI \
	CONFIG_DRM_DW_HDMI_CEC \
	CONFIG_ROCKCHIP_ANALOGIX_DP=n \
	CONFIG_ROCKCHIP_CDN_DP=n \
	CONFIG_ROCKCHIP_DW_HDMI=y \
	CONFIG_ROCKCHIP_DW_MIPI_DSI=y \
	CONFIG_ROCKCHIP_INNO_HDMI=y \
	CONFIG_ROCKCHIP_LVDS=y \
	CONFIG_ROCKCHIP_RGB=n \
	CONFIG_ROCKCHIP_RK3066_HDMI=n \
	CONFIG_ROCKCHIP_VOP=y \
	CONFIG_ROCKCHIP_VOP2=y \
	CONFIG_DRM_GEM_CMA_HELPER \
	CONFIG_DRM_PANEL=y \
	CONFIG_DRM_PANEL_BRIDGE=y \
	CONFIG_DRM_PANEL_SIMPLE
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi-cec.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-mipi-dsi.ko \
	$(LINUX_DIR)/drivers/media/cec/cec.ko@lt5.10 \
	$(LINUX_DIR)/drivers/media/cec/core/cec.ko@ge5.10 \
	$(LINUX_DIR)/drivers/phy/rockchip/phy-rockchip-inno-hdmi.ko \
	$(LINUX_DIR)/drivers/gpu/drm/drm_cma_helper.ko@ge6.0 \
	$(LINUX_DIR)/drivers/gpu/drm/panel/panel-simple.ko \
	$(LINUX_DIR)/drivers/gpu/drm/rockchip/rockchipdrm.ko
  AUTOLOAD:=$(call AutoProbe,rockchipdrm phy-rockchip-inno-hdmi dw-hdmi-cec)
endef

define KernelPackage/drm-rockchip/description
  Direct Rendering Manager (DRM) support for Rockchip
endef

$(eval $(call KernelPackage,drm-rockchip))

define KernelPackage/saradc-rockchip
  SUBMENU:=$(IIO_MENU)
  TITLE:=Rockchip SARADC support
  DEPENDS:=@TARGET_rockchip +kmod-industrialio-triggered-buffer
  KCONFIG:= \
	CONFIG_RESET_CONTROLLER=y \
	CONFIG_ROCKCHIP_SARADC
  FILES:= \
	$(LINUX_DIR)/drivers/iio/adc/rockchip_saradc.ko
  AUTOLOAD:=$(call AutoProbe,rockchip_saradc)
endef

define KernelPackage/saradc-rockchip/description
  Support for the SARADC found in SoCs from Rockchip
endef

$(eval $(call KernelPackage,saradc-rockchip))
