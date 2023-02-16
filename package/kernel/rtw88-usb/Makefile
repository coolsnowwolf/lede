#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtw88-usb
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-02-11
PKG_SOURCE_URL:=https://github.com/lwfinger/rtw88.git
PKG_SOURCE_VERSION:=3af004dd40588e86280f0c58a483de6f53b9803f
PKG_MIRROR_HASH:=13a476c6e46702dee0a3ea2cd743587ab889414be2dce979e652f3bbfa106d76

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtw88-default
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtw88 family usb driver
  DEPENDS:=+kmod-mac80211 +kmod-usb-core \
	+@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
endef

define KernelPackage/rtw88-usb
  $(KernelPackage/rtw88-default)
  HIDDEN:=1
  FILES:= \
	$(PKG_BUILD_DIR)/rtw_core.ko \
	$(PKG_BUILD_DIR)/rtw88_usb.ko
endef

define KernelPackage/rtl8723du
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8723DU support
  DEPENDS+=+kmod-rtw88-usb +rtl8723du-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw_8723d.ko \
	$(PKG_BUILD_DIR)/rtw88_8723du.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8723du)
endef

define KernelPackage/rtl8821cu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8821CU support
  DEPENDS+=+kmod-rtw88-usb +rtl8821ce-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw_8821c.ko \
	$(PKG_BUILD_DIR)/rtw88_8821cu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8821cu)
endef

define KernelPackage/rtl8822bu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8822BU support
  DEPENDS+=+kmod-rtw88-usb +rtl8822be-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw_8822b.ko \
	$(PKG_BUILD_DIR)/rtw88_8822bu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8822bu)
endef

define KernelPackage/rtl8822cu
  $(KernelPackage/rtw88-default)
  TITLE:=Realtek RTL8822CU support
  DEPENDS+=+kmod-rtw88-usb +rtl8822ce-firmware
  FILES:= \
	$(PKG_BUILD_DIR)/rtw_8822c.ko \
	$(PKG_BUILD_DIR)/rtw88_8822cu.ko
  AUTOLOAD:=$(call AutoProbe,rtw88_8822cu)
endef

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/autoconf.h \
	-include backport/backport.h

NOSTDINC_FLAGS+=-DBUILD_OPENWRT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,rtw88-usb))
$(eval $(call KernelPackage,rtl8723du))
$(eval $(call KernelPackage,rtl8821cu))
$(eval $(call KernelPackage,rtl8822bu))
$(eval $(call KernelPackage,rtl8822cu))
