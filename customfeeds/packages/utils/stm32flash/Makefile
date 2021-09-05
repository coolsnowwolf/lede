#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stm32flash
PKG_VERSION:=0.6
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_HASH:=ee9b40d4d3e5cd28b993e08ae2a2c3c559b6bea8730cd7e1d40727dedb1dda09

PKG_MAINTAINER:=Christian Pointner <equinox@spreadspace.org>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=gpl-2.0.txt

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/stm32flash
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Microcontroller programming
  URL:=https://sourceforge.net/projects/stm32flash
  TITLE:=Firmware flash tool for STM32's serial bootloader
endef

define Package/stm32flash/description
  This tool can be used to burn firmware images to STM32 ARM processors
  using the built-in serial bootloader.
endef

define Package/stm32flash/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stm32flash $(1)/usr/bin/
endef

$(eval $(call BuildPackage,stm32flash))
