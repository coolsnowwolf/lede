#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mwlwifi
PKG_RELEASE=1

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/kaloz/mwlwifi
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2018-03-30
PKG_SOURCE_VERSION:=fcaea79ad33d6ae3c381d9e96bf77d6870ca8e79
PKG_MIRROR_HASH:=1c0fa04ca80939038139dd50a50d9dc0d879b7cb594770282e3ec0008a479452

PKG_MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/mwlwifi
  SUBMENU:=Wireless Drivers
  TITLE:=Marvell 88W8864/88W8897/88W8964 wireless driver
  DEPENDS:=+kmod-mac80211 +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11W_SUPPORT @PCI_SUPPORT @TARGET_mvebu
  FILES:=$(PKG_BUILD_DIR)/mwlwifi.ko
  AUTOLOAD:=$(call AutoLoad,50,mwlwifi)
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
		$(KERNEL_MAKE_FLAGS) \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Package/mwlwifi-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Marvell $(1) firmware
  DEPENDS:=+kmod-mwlwifi @TARGET_mvebu
endef

define Package/mwlwifi-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/lib/firmware/mwlwifi
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/$(2) $(1)/lib/firmware/mwlwifi/
	$(CP) $(PKG_BUILD_DIR)/bin/firmware/Marvell_license.txt $(1)/lib/firmware/mwlwifi/$(2).Marvell_license.txt
endef

define Package/mwlwifi-firmware-88w8864
$(call Package/mwlwifi-firmware-default,88W8864)
endef

define Package/mwlwifi-firmware-88w8864/install
	$(call Package/mwlwifi-firmware/install,$(1),88W8864.bin)
endef

define Package/mwlwifi-firmware-88w8897
$(call Package/mwlwifi-firmware-default,88W8897)
endef

define Package/mwlwifi-firmware-88w8897/install
	$(call Package/mwlwifi-firmware/install,$(1),88W8897.bin)
endef

define Package/mwlwifi-firmware-88w8964
$(call Package/mwlwifi-firmware-default,88W8964)
endef

define Package/mwlwifi-firmware-88w8964/install
	$(call Package/mwlwifi-firmware/install,$(1),88W8964.bin)
endef

$(eval $(call KernelPackage,mwlwifi))
$(eval $(call BuildPackage,mwlwifi-firmware-88w8864))
$(eval $(call BuildPackage,mwlwifi-firmware-88w8897))
$(eval $(call BuildPackage,mwlwifi-firmware-88w8964))
