#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sangam_atm

ifeq ($(CONFIG_AR7_ATM_FW_VERSION_705),y)
PKG_VERSION:=D7.05.01.00
PKG_HASH:=874b5baff92159e99cef0dfb839d6fdc124a46aef7374653064d29d9eee9a3c9
PKG_RELEASE:=R1
PATCH_DIR:=patches-$(PKG_VERSION)
endif

ifeq ($(CONFIG_AR7_ATM_FW_VERSION_704),y)
PKG_VERSION:=D7.04.03.00
PKG_HASH:=af2e203e4c86f9a86cdd07d172897b07e66a25379376e8da4c1b14816f86b58f
PKG_RELEASE:=R1
PATCH_DIR:=patches-$(PKG_VERSION)
endif

ifeq ($(CONFIG_AR7_ATM_FW_VERSION_703),y)
PKG_VERSION:=D7.03.01.00
PKG_HASH:=2d156164ef5abbe10ffb2400ada4cef2035df3ba2b3f866f00bdede489861c4d
PKG_RELEASE:=R2
PATCH_DIR:=patches-D7.04.03.00
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE).tar.bz2
PKG_SOURCE_URL:=http://mirror2.openwrt.org/sources

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sangam-atm/Default
  SUBMENU:=Network Devices
  DEPENDS:=@TARGET_ar7_generic +kmod-atm
  TITLE:=AR7 ADSL driver
  FILES:=$(PKG_BUILD_DIR)/tiatm.ko
  AUTOLOAD:=$(call AutoLoad,50,tiatm)
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
  MENU:=1
endef

define KernelPackage/sangam-atm/config
	source "$(SOURCE)/Config.in"
endef

define KernelPackage/sangam-atm-annex-a
$(call KernelPackage/sangam-atm/Default)
  TITLE+= (Annex A, ADSL over POTS)
endef

define KernelPackage/sangam-atm-annex-a/description
 The AR7 ADSL driver for Annex A (ADSL over POTS).
endef

define KernelPackage/sangam-atm-annex-a/config
$(call KernelPackage/sangam-atm/config)
endef

define KernelPackage/sangam-atm-annex-b
$(call KernelPackage/sangam-atm/Default)
  TITLE+= (Annex B, ADSL over ISDN)
endef

define KernelPackage/sangam-atm-annex-b/description
 The AR7 ADSL driver for Annex B (ADSL over ISDN).
endef

define KernelPackage/sangam-atm-annex-a/config
$(call KernelPackage/sangam-atm/config)
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		modules
endef

define KernelPackage/sangam-atm-annex-a/install
	mkdir -p $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ar0700mp.bin $(1)/lib/firmware/
	$(LN) ar0700mp.bin $(1)/lib/firmware/ar0700xx.bin
endef

define KernelPackage/sangam-atm-annex-b/install
	mkdir -p $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ar0700db.bin $(1)/lib/firmware/
	$(LN) ar0700db.bin $(1)/lib/firmware/ar0700xx.bin
endef

$(eval $(call KernelPackage,sangam-atm-annex-a))
$(eval $(call KernelPackage,sangam-atm-annex-b))
