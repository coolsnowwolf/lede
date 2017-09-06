#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtc-rv5c386a
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtc-rv5c386a
  SUBMENU:=Other modules
  DEPENDS:=@TARGET_brcm47xx @!IN_SDK
  TITLE:=Driver for RTC RV5C386A (used in WL-700gE and WL-HDD)
  AUTOLOAD:=$(call AutoLoad,70,rtc)
  FILES:=$(PKG_BUILD_DIR)/rtc.ko
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(BUILDFLAGS)" \
		modules
endef

$(eval $(call KernelPackage,rtc-rv5c386a))
