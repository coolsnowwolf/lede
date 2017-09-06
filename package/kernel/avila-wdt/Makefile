#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=avila-wdt
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/avila-wdt
  SUBMENU:=Other modules
  TITLE:=GPIO hardware watchdog driver for modified Avila boards
  DEPENDS:=@GPIO_SUPPORT @TARGET_ixp4xx
  FILES:=$(PKG_BUILD_DIR)/avila-wdt.ko
  AUTOLOAD:=$(call AutoLoad,10,avila-wdt)
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,avila-wdt))
