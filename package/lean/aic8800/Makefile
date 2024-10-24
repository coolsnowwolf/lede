# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2023-2024 AnYun
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aic8800
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2024-03-04
PKG_SOURCE_URL:=https://github.com/radxa-pkg/aic8800.git
PKG_SOURCE_VERSION:=6a7ec8536a1688a785740d98ca96a4d00b274c56
PKG_MIRROR_HASH:=75bbd6f410089d8301663aebdeeb79027420ec45dd054b5851c408dfeef23776

MAKE_PATH:=src/SDIO/driver_fw/driver/aic8800
PKG_EXTMOD_SUBDIRS:=$(MAKE_PATH)

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/aic8800
  SUBMENU:=Wireless Drivers
  TITLE:=SKI WB800D80S wireless support
  DEPENDS:=+kmod-mac80211 +kmod-mmc \
	+@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:= \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic8800_bsp/aic8800_bsp.ko \
	$(PKG_BUILD_DIR)/$(MAKE_PATH)/aic8800_fdrv/aic8800_fdrv.ko
  AUTOLOAD:=$(call AutoProbe,aic8800_bsp aic8800_fdrv)
  MODPARAMS.aic8800_fdrv:=he_on=n
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

define KernelPackage/aic8800/install
	$(INSTALL_DIR) $(1)/lib/firmware/aic8800
	$(CP) $(PKG_BUILD_DIR)/src/SDIO/driver_fw/fw/aic8800D80/* $(1)/lib/firmware/aic8800
endef

$(eval $(call KernelPackage,aic8800))
