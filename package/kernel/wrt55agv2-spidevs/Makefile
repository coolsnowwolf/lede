#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wrt55agv2-spidevs
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/wrt55agv2-spidevs
  SUBMENU:=Other modules
  TITLE:=WRT55AG v2 SPI devices support
  DEPENDS:=@TARGET_ath25 +kmod-spi-gpio-old +kmod-spi-ks8995
  FILES:=$(PKG_BUILD_DIR)/wrt55agv2_spidevs.ko
endef

define KernelPackage/wrt55agv2-spidevs/description
  Kernel module for the SPI devices on the WRT55AG v2 board.
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,wrt55agv2-spidevs))
