#
# Copyright (C) 2019 Jianhui Zhao
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-ev
PKG_VERSION:=1.5
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/brimworks/lua-ev/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=26ac116722a241bf59daf5315ce0ffe751c1babea9a146ffc0a389f1af3facca

PKG_MAINTAINER:=Jianhui Zhao <jianhuizhao329@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-ev
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=lua-ev
  URL:=https://github.com/brimworks/lua-ev
  DEPENDS:=+lua +libev
endef

define Package/lua-ev/description
  Lua integration with libev.
endef

define Package/lua-ev/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/share/lua/cmod/ev.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-ev))
