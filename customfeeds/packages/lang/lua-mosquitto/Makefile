#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-mosquitto
PKG_VERSION:=0.4.1
PKG_RELEASE:=1
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_MIRROR_HASH:=d6bc828ea05f9a1ff3241f575a6ce699e577f9b79277182699e8fa03ac094ac9
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/flukso/lua-mosquitto.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/lua-mosquitto
    SUBMENU:=Lua
    SECTION:=lang
    CATEGORY:=Languages
    TITLE:=Lua-mosquitto
    DEPENDS:=+libmosquitto +lua
    MAINTAINER:=Karl Palsson <karlp@etactica.com>
endef

define Package/lua-mosquitto/description
	Lua bindings to libmosquitto
endef

define Package/lua-mosquitto/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mosquitto.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-mosquitto))
