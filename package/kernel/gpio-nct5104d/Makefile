#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio-nct5104d
PKG_RELEASE:=1

PKG_MAINTAINER:=Florian Eckert <Eckert.Florian@googlemail.com>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio-nct5104d
  SUBMENU:=Other modules
  TITLE:= GPIO nct5104d support
  DEPENDS:= @GPIO_SUPPORT @TARGET_x86
  FILES:=$(PKG_BUILD_DIR)/gpio-nct5104d.ko
  AUTOLOAD:=$(call AutoLoad,30,gpio-nct5104d,1)
  KCONFIG:=
endef

define KernelPackage/gpio-nct5104d/description
  Support for GPIO functionality of NCT5104D super I/O chip.
endef

define Build/Compile
	 $(MAKE) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		modules
endef

$(eval $(call KernelPackage,gpio-nct5104d))
