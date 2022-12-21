#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=leds-ws2812b
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/leds-ws2812b
  SUBMENU:=LED modules
  TITLE:=Worldsemi WS2812B (NeoPixel) LED support
  FILES:= \
	$(PKG_BUILD_DIR)/leds-ws2812b.ko
  AUTOLOAD:=$(call AutoProbe,leds-ws2812b,1)
  DEPENDS:=@TARGET_mediatek_filogic
endef

define KernelPackage/leds-ws2812b/description
  LED support for driving WS2812B (NeoPixel) using SPI MOSI.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,leds-ws2812b))
