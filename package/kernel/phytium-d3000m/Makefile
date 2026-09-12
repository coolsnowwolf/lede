# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=phytium-d3000m
PKG_VERSION:=2026.09.03
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>
PKG_BUILD_PARALLEL:=1
PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_kmod-phytium-dc CONFIG_PACKAGE_kmod-phytium-gpu \
	CONFIG_PACKAGE_kmod-phytium-vpu CONFIG_PACKAGE_kmod-phytium-npu

include $(INCLUDE_DIR)/package.mk

define KernelPackage/phytium-dc
  SUBMENU:=Video Support
  TITLE:=Phytium FTD330 display controller
  DEPENDS:=@TARGET_phytium @DISPLAY_SUPPORT +kmod-drm +kmod-drm-kms-helper \
	+kmod-backlight +LINUX_6_18:kmod-drm-display-helper \
	+LINUX_6_18:kmod-drm-client-lib
  KCONFIG:=CONFIG_DRM_DISPLAY_DP_HELPER=y CONFIG_DRM_DISPLAY_HDCP_HELPER=y \
	CONFIG_DRM_FBDEV_EMULATION=y
  FILES:=$(PKG_BUILD_DIR)/dc_driver/phytium/ftd330-drm-dc.ko
  AUTOLOAD:=$(call AutoProbe,ftd330-drm-dc)
endef

define KernelPackage/phytium-gpu
  SUBMENU:=Video Support
  TITLE:=Phytium FTG340 GPU
  DEPENDS:=@TARGET_phytium +kmod-drm
  KCONFIG:=CONFIG_SYNC_FILE=y
  FILES:=$(PKG_BUILD_DIR)/gpu_driver/ftg340.ko
  AUTOLOAD:=$(call AutoProbe,ftg340)
endef

define KernelPackage/phytium-vpu
  SUBMENU:=Video Support
  TITLE:=Phytium FTV310 video codec
  DEPENDS:=@TARGET_phytium +kmod-drm
  KCONFIG:=CONFIG_SYNC_FILE=y
  FILES:=$(PKG_BUILD_DIR)/vpu_driver/ftv310_vpu_driver.ko
  AUTOLOAD:=$(call AutoProbe,ftv310_vpu_driver)
endef

define KernelPackage/phytium-npu
  SUBMENU:=Other modules
  TITLE:=Phytium NPU and DMA buffer heap
  DEPENDS:=@TARGET_phytium +kmod-dma-buf
  KCONFIG:=CONFIG_DMABUF_HEAPS=y
  FILES:=$(PKG_BUILD_DIR)/npu_driver/phytium_npu.ko \
	$(PKG_BUILD_DIR)/npu_driver/phytium_npu_platform.ko \
	$(PKG_BUILD_DIR)/npu_driver/phytium_npu_pci.ko \
	$(PKG_BUILD_DIR)/npu_driver/phytium_npu_heap.ko
  AUTOLOAD:=$(call AutoLoad,50,phytium_npu_heap phytium_npu_platform phytium_npu_pci,1)
endef

VPU_CFLAGS=$(if $(shell grep -l "int dma_resv_reserve_fences" $(LINUX_DIR)/include/linux/dma-resv.h 2>/dev/null),-DPHYTIUM_DMA_RESV_RESERVE_FENCES)

DC_CONFIGS:=CHIP_FTD330 DEC MMU NCC CHANGE_PIXCLK FBDEV_ON LANE_TRAIN \
	POWER_OPERATION EDP_BL PSR LOW_FPS

define Build/Compile
	$(if $(CONFIG_PACKAGE_kmod-phytium-dc),$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/dc_driver \
		CONFIG_DRM_PHYTIUM=m $(foreach c,$(DC_CONFIGS),CONFIG_PHYTIUM_$(c)=y) \
		EXTRA_CFLAGS="$(foreach c,$(DC_CONFIGS),-DCONFIG_PHYTIUM_$(c))" modules)
	$(if $(CONFIG_PACKAGE_kmod-phytium-gpu),$(MAKE) -C $(PKG_BUILD_DIR)/gpu_driver \
		$(KERNEL_MAKE_FLAGS) KERNEL_DIR=$(LINUX_DIR) ARCH_TYPE=$(LINUX_KARCH) all)
	$(if $(CONFIG_PACKAGE_kmod-phytium-vpu),$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/vpu_driver EXTRA_CFLAGS="$(VPU_CFLAGS)" modules)
	$(if $(CONFIG_PACKAGE_kmod-phytium-npu),$(KERNEL_MAKE) M=$(PKG_BUILD_DIR)/npu_driver BUILD_HEAP=y modules)
endef

$(eval $(call KernelPackage,phytium-dc))
$(eval $(call KernelPackage,phytium-gpu))
$(eval $(call KernelPackage,phytium-vpu))
$(eval $(call KernelPackage,phytium-npu))
