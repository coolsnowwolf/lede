#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio-button-hotplug
PKG_RELEASE:=3
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio-button-hotplug
  SUBMENU:=Other modules
  TITLE:=Simple GPIO Button Hotplug driver
  FILES:=$(PKG_BUILD_DIR)/gpio-button-hotplug.ko
  AUTOLOAD:=$(call AutoLoad,30,gpio-button-hotplug,1)
  KCONFIG:=
endef

define KernelPackage/gpio-button-hotplug/description
 This is a replacement for the following in-kernel drivers:
 1) gpio_keys (KEYBOARD_GPIO)
 2) gpio_keys_polled (KEYBOARD_GPIO_POLLED)

 Instead of generating input events (like in-kernel drivers do) it generates
 uevent-s and broadcasts them. This allows disabling input subsystem which is
 an overkill for OpenWrt simple needs.
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,gpio-button-hotplug))
