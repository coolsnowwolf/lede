#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=w1-gpio-custom
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/w1-gpio-custom
  SUBMENU:=W1 support
  TITLE:=Custom GPIO-based 1-wire device
  DEPENDS:=kmod-w1 +kmod-w1-master-gpio
  FILES:=$(PKG_BUILD_DIR)/w1-gpio-custom.ko
  KCONFIG:=
endef

define KernelPackage/w1-gpio-custom/description
 Kernel module to register a custom w1-gpio platform device.
endef

EXTRA_KCONFIG:= \
	CONFIG_W1_MASTER_GPIO_CUSTOM=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG))))

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,w1-gpio-custom))

