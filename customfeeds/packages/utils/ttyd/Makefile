#
# Copyright (C) 2016 Shuanglei Tao <tsl0922@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ttyd
PKG_VERSION:=1.6.3
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/tsl0922/ttyd/tar.gz/$(PKG_VERSION)?
PKG_HASH:=1116419527edfe73717b71407fb6e06f46098fc8a8e6b0bb778c4c75dc9f64b9

PKG_MAINTAINER:=Shuanglei Tao <tsl0922@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/ttyd
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Command-line tool for sharing terminal over the web
	DEPENDS:=+libopenssl +libjson-c +libuv +zlib +libwebsockets-full
	URL:=https://github.com/tsl0922/ttyd
	SUBMENU:=Terminal
endef

define Package/ttyd/description
ttyd is a command-line tool for sharing terminal over the web.
endef

define Package/ttyd/conffiles
/etc/config/ttyd
endef

define Package/ttyd/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ttyd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/ttyd.config $(1)/etc/config/ttyd
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/ttyd.init $(1)/etc/init.d/ttyd
endef

$(eval $(call BuildPackage,ttyd))
