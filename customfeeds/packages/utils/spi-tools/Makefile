#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spi-tools
PKG_VERSION:=0.8.7
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/cpb-/spi-tools/tar.gz/$(PKG_VERSION)?
PKG_HASH:=550f505cc5c34a50d5cd36c49c69b2709fca3f0be4f0777e3f96a45c1ffdbd79

PKG_MAINTAINER:=John Crispin <blogic@openwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_ASLR_PIE:=0

include $(INCLUDE_DIR)/package.mk

define Package/spi-tools
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Command line SPI tools
  URL:=https://github.com/cpb-/spi-tools
endef

define Package/spi-tools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/spi-config $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/spi-pipe $(1)/usr/bin
endef

$(eval $(call BuildPackage,spi-tools))
