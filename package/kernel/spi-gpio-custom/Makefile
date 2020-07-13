#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=spi-gpio-custom
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/spi-gpio-custom
  SUBMENU:=SPI Support
  TITLE:=Custom GPIO-based SPI device
  DEPENDS:=@GPIO_SUPPORT +kmod-spi-bitbang +kmod-spi-gpio +kmod-spi-dev @LINUX_4_14
  FILES:=$(PKG_BUILD_DIR)/spi-gpio-custom.ko
  KCONFIG:=
endef

define KernelPackage/spi-gpio-custom/description
 Kernel module for register a custom spi-gpio platform device.
endef

EXTRA_KCONFIG:= \
	CONFIG_SPI_GPIO_CUSTOM=m

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

$(eval $(call KernelPackage,spi-gpio-custom))
