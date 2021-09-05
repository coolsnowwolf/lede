include $(TOPDIR)/rules.mk

PKG_NAME:=lua-libmodbus
PKG_VERSION:=0.7
PKG_RELEASE:=1

PKG_SOURCE:=lua-libmodbus-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=lua-libmodbus-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/etactica/lua-libmodbus/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=70369f4c4e36421095bf00dcee36c633b4284eaa79bcfdfd230bfefb27acefda
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_MAINTAINER:=Karl Palsson <karlp@etactica.com>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/lua-libmodbus
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=lua-libmodbus
  URL:=https://github.com/etactica/lua-libmodbus
  DEPENDS:=+liblua +libmodbus
endef

define Package/lua-libmodbus/description
	lua-libmodbus is a binding to libmodbus,
	see also http://www.libmodbus.org
endef

define Build/Configure
endef

define Package/lua-libmodbus/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmodbus.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,lua-libmodbus))
