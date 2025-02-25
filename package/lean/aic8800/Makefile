# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2023-2025 AnYun
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aic8800
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2024-06-20
PKG_SOURCE_URL:=https://github.com/radxa-pkg/aic8800.git
PKG_SOURCE_VERSION:=8ad987876a6220fffb40a755e0454250ce2c5fdd
PKG_MIRROR_HASH:=4a22e28a4341eb7516f03ea7e6c1c0523013e1aaf72c17428eb3749c8f553856

ifeq ($(BUILD_VARIANT),sdio)
MAKE_PATH:=src/SDIO/driver_fw/driver/aic8800
PATCH_DIR:=$(CURDIR)/patches-sdio
else
MAKE_PATH:=src/USB/driver_fw/drivers/aic8800
PATCH_DIR:=$(CURDIR)/patches-usb
endif

PKG_EXTMOD_SUBDIRS:=$(MAKE_PATH)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/aic8800s
  SUBMENU:=Wireless Drivers
  TITLE:=AIC8800 SDIO wireless support
  DEPENDS:=+kmod-mac80211 +kmod-mmc \
	+@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:= \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic8800_bsp/aic8800_bsp.ko \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic8800_fdrv/aic8800_fdrv.ko
  AUTOLOAD:=$(call AutoProbe,aic8800_bsp aic8800_fdrv)
  MODPARAMS.aic8800_fdrv:=he_on=n
  VARIANT:=sdio
endef

define KernelPackage/aic8800u
  SUBMENU:=Wireless Drivers
  TITLE:=AIC8800 USB wireless support
  DEPENDS:=+kmod-mac80211 +kmod-usb-core @USB_SUPPORT \
	+@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:= \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic_load_fw/aic_load_fw.ko \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic8800_fdrv/aic8800_fdrv_usb.ko
  AUTOLOAD:=$(call AutoProbe,aic_load_fw aic8800_fdrv_usb)
  MODPARAMS.aic8800_fdrv_usb:=he_on=n
  VARIANT:=usb
endef

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/backport.h

EXTRA_CFLAGS:= -DBUILD_OPENWRT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)/$(MAKE_PATH)/" \
		modules
endef

define KernelPackage/aic8800s/install
	$(INSTALL_DIR) $(1)/lib/firmware/aic8800
	$(CP) $(PKG_BUILD_DIR)/src/SDIO/driver_fw/fw/aic8800D80/* $(1)/lib/firmware/aic8800
endef

define KernelPackage/aic8800u/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) $(PKG_BUILD_DIR)/src/USB/driver_fw/fw/* $(1)/lib/firmware
endef

$(eval $(call KernelPackage,aic8800s))
$(eval $(call KernelPackage,aic8800u))
