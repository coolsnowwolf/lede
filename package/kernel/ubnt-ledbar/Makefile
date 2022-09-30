#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ubnt-ledbar
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/leds-ubnt-ledbar
  SUBMENU:=LED modules
  TITLE:=Ubiquiti UniFi 6 LR LED support
  FILES:= \
	$(PKG_BUILD_DIR)/leds-ubnt-ledbar.ko
  AUTOLOAD:=$(call AutoProbe,leds-ubnt-ledbar,1)
  DEPENDS:=+kmod-i2c-core
endef

define KernelPackage/leds-ubnt-ledbar/description
  LED support for some Ubiquiti UniFi access points.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,leds-ubnt-ledbar))
