#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mwlwifi
PKG_VERSION:=10.3.4.0-20171011
PKG_RELEASE=1

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/kaloz/mwlwifi
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=e119077b68d64e368cb9cc46bd364308db4289dc
PKG_MIRROR_HASH:=6b450f91941de1787a41e1cf162764e9b8e9ff9cdf7f80743afc5081ef435c81

PKG_MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/mwlwifi
  SUBMENU:=Wireless Drivers
  TITLE:=Marvell 88W8864 wireless driver
  DEPENDS:=+kmod-mac80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11W_SUPPORT @PCI_SUPPORT @TARGET_mvebu
  FILES:=$(PKG_BUILD_DIR)/mwlwifi.ko
  AUTOLOAD:=$(call AutoLoad,50,mac80211 mwlwifi)
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/backport.h

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define KernelPackage/mwlwifi/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/lib/firmware/mwlwifi
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/88W8864.bin $(1)/lib/firmware/mwlwifi/
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/88W8897.bin $(1)/lib/firmware/mwlwifi/
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/88W8964.bin $(1)/lib/firmware/mwlwifi/
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/Marvell_license.txt $(1)/lib/firmware/mwlwifi/
endef

$(eval $(call KernelPackage,mwlwifi))
