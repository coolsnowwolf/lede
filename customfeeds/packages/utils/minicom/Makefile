#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minicom
PKG_VERSION:=2.7.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://alioth.debian.org/frs/download.php/file/4215
PKG_HASH:=532f836b7a677eb0cb1dca8d70302b73729c3d30df26d58368d712e5cca041f1
PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:minicom:minicom

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/package.mk

define Package/minicom
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +libncurses
  TITLE:=Terminal emulation program
  URL:=http://alioth.debian.org/projects/minicom/
  SUBMENU:=Terminal
endef

define Package/minicom/conffiles
/etc/minirc.dfl
endef

define Package/minicom/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ascii-xfr $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,minicom))
