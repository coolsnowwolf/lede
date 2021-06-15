#
# Copyright (C) 2021 ImmortalWrt
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8189es
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/jwrdegoede/rtl8189ES_linux.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-03-03
PKG_SOURCE_VERSION:=03ac413135a355b55b693154c44b70f86a39732e
PKG_MIRROR_HASH:=5cdd69a0c2383e66ae27b0518b4d6a3e845928767d644e1e2b12095b98ed8fbe

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8189es
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8189ETV SDIO WiFi
  DEPENDS:=+kmod-cfg80211 +kmod-mmc +@DRIVER_11N_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8189es.ko
  AUTOLOAD:=$(call AutoProbe,rtl8189es)
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/autoconf.h \
	-include backport/backport.h

EXTRA_KCONFIG:= \
	CONFIG_RTL8189ES=m \
	USER_MODULE_NAME=rtl8189es

EXTRA_CFLAGS:= \
	-DRTW_SINGLE_WIPHY \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_IOCTL_CFG80211 \
	-DBUILD_OPENWRT

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
	USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define KernelPackage/rtl8189es/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/50_rtl-wifi $(1)/etc/uci-defaults/
endef

$(eval $(call KernelPackage,rtl8189es))
