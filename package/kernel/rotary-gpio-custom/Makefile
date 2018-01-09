#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rotary-gpio-custom
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rotary-gpio-custom
  SUBMENU:=Other modules
  TITLE:=Custom GPIO-based rotary encoder device
  DEPENDS:=@GPIO_SUPPORT +kmod-input-gpio-encoder @LINUX_3_18||LINUX_4_4
  FILES:=$(PKG_BUILD_DIR)/rotary-gpio-custom.ko
  KCONFIG:=
endef

define KernelPackage/rotary-gpio-custom/description
 Kernel module for register a custom rotary-gpio-encoder platform device.
endef

EXTRA_KCONFIG:= \
	CONFIG_ROTARY_GPIO_CUSTOM=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,rotary-gpio-custom))
