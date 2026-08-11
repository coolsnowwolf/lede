# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 OpenWrt.org

define KernelPackage/drm-rockchip
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Rockchip DRM support
  DEPENDS:=@TARGET_rockchip \
	+kmod-backlight +kmod-cec-core \
	+LINUX_6_18:kmod-drm-client-lib \
	+kmod-drm-display-helper \
	+kmod-drm-dma-helper \
	+kmod-multimedia-input
  KCONFIG:= \
	CONFIG_HDMI=y \
	CONFIG_DRM_BRIDGE=y \
	CONFIG_DRM_DW_HDMI \
	CONFIG_DRM_DW_HDMI_CEC \
	CONFIG_DRM_FBDEV_EMULATION=y \
	CONFIG_DRM_FBDEV_OVERALLOC=100 \
	CONFIG_DRM_LOAD_EDID_FIRMWARE=y \
	CONFIG_DRM_PANEL_BRIDGE=y \
	CONFIG_DRM_PANEL_SIMPLE \
	CONFIG_DRM_ROCKCHIP \
	CONFIG_PHY_ROCKCHIP_INNO_HDMI \
	CONFIG_ROCKCHIP_ANALOGIX_DP=n \
	CONFIG_ROCKCHIP_CDN_DP=n \
	CONFIG_ROCKCHIP_DW_HDMI=y \
	CONFIG_ROCKCHIP_DW_HDMI_QP=y \
	CONFIG_ROCKCHIP_DW_MIPI_DSI=n \
	CONFIG_ROCKCHIP_INNO_HDMI=y \
	CONFIG_ROCKCHIP_LVDS=n \
	CONFIG_ROCKCHIP_RGB=n \
	CONFIG_ROCKCHIP_RK3066_HDMI=n \
	CONFIG_ROCKCHIP_VOP=y \
	CONFIG_ROCKCHIP_VOP2=y
  FILES:= \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi-cec.ko \
	$(LINUX_DIR)/drivers/gpu/drm/bridge/synopsys/dw-hdmi-qp.ko \
	$(LINUX_DIR)/drivers/gpu/drm/drm_dp_aux_bus.ko@lt5.19 \
	$(LINUX_DIR)/drivers/gpu/drm/panel/panel-simple.ko \
	$(LINUX_DIR)/drivers/gpu/drm/rockchip/rockchipdrm.ko \
	$(LINUX_DIR)/drivers/phy/rockchip/phy-rockchip-inno-hdmi.ko
  AUTOLOAD:=$(call AutoProbe,dw-hdmi-cec phy-rockchip-inno-hdmi rockchipdrm,1)
endef

define KernelPackage/drm-rockchip/description
  Direct Rendering Manager (DRM) support for Rockchip
endef

$(eval $(call KernelPackage,drm-rockchip))

define KernelPackage/drm-rocket
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Rockchip NPU support
  DEPENDS:=@TARGET_rockchip @LINUX_6_18 \
	+kmod-drm-sched +kmod-drm-shmem-helper
  KCONFIG:= \
	CONFIG_DRM_ACCEL=y \
	CONFIG_DRM_ACCEL_ROCKET
  FILES:=$(LINUX_DIR)/drivers/accel/rocket/rocket.ko
  AUTOLOAD:=$(call AutoProbe,rocket)
endef

$(eval $(call KernelPackage,drm-rocket))
