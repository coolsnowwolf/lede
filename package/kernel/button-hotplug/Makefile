#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=button-hotplug
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/button-hotplug
  SUBMENU:=Other modules
  TITLE:=Button Hotplug driver
  DEPENDS:=+kmod-input-core
  FILES:=$(PKG_BUILD_DIR)/button-hotplug.ko
  AUTOLOAD:=$(call AutoLoad,30,button-hotplug,1)
  KCONFIG:=
endef

define KernelPackage/button-hotplug/description
  Kernel module to generate button uevent-s from input subsystem events.
  If your device uses GPIO buttons, see gpio-button-hotplug.
endef

EXTRA_KCONFIG:= \
	CONFIG_BUTTON_HOTPLUG=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,button-hotplug))
