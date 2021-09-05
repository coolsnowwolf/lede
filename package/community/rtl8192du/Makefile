#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8192du
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lwfinger/rtl8192du
PKG_SOURCE_DATE:=2020-12-12
PKG_SOURCE_VERSION:=331ec03d89d0c0ebe771c4011846f029f6af33d3
PKG_MIRROR_HASH:=f0acb833dd51cde41b982be3d341356548c7454d33afecbd25b2e03dea6d53b0

PKG_BUILD_PARALLEL:=1
PKG_MAINTAINER:=Larry Finger <Larry.Finger@lwfinger.net>

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192du
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8192DU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT
  FILES:= $(PKG_BUILD_DIR)/8192du.ko
  AUTOLOAD:=$(call AutoProbe,8192du)
endef

define KernelPackage/rtl8192du/description
  Kernel modules for the Realtek 8192DU USB 802.11n wireless USB adapters
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

NOSTDINC_FLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_CONCURRENT_MODE -DBUILD_OPENWRT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(LINUX_DIR) \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8192DU=m \
		modules
endef

define KernelPackage/rtl8192du/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtlwifi
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw.bin $(1)/lib/firmware/rtlwifi/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw_wol.bin $(1)/lib/firmware/rtlwifi/
endef

$(eval $(call KernelPackage,rtl8192du))
