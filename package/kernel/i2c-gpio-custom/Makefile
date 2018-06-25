#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=i2c-gpio-custom
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define KernelPackage/i2c-gpio-custom
  SUBMENU:=I2C support
  TITLE:=Custom GPIO-based I2C device
  DEPENDS:=@GPIO_SUPPORT +kmod-i2c-core +kmod-i2c-gpio
  FILES:=$(PKG_BUILD_DIR)/i2c-gpio-custom.ko
  KCONFIG:=
endef

define KernelPackage/i2c-gpio-custom/description
 Kernel module for register a custom i2c-gpio platform device.
endef

EXTRA_KCONFIG:= \
	CONFIG_I2C_GPIO_CUSTOM=m

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

$(eval $(call KernelPackage,i2c-gpio-custom))
