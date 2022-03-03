#
# Copyright (C) 2020 Mauri Sandberg <sandberg@mailfence.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio-nxp-74hc153
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio-nxp-74hc153
  SUBMENU:=Other modules
  TITLE:= NXP 74HC153 GPIO expander
  FILES:=$(PKG_BUILD_DIR)/gpio-nxp-74hc153.ko
  AUTOLOAD:=$(call AutoLoad,30,gpio-nxp-74hc153,1)
  KCONFIG:=
  DEPENDS:= @GPIO_SUPPORT @TARGET_ath79
endef

define KernelPackage/gpio-nxp-74hc153/description
  Platform driver for NXP 74HC153 Dual 4-input Multiplexer.
  This provides a GPIO interface supporting input mode only.
endef

define Build/Compile
	$(KERNEL_MAKE) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,gpio-nxp-74hc153))
