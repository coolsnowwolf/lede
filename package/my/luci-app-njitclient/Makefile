include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-njitclient
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-njitclient
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=NJIT 802.1X Client for LuCI
	DEPENDS:=+njit8021xclient
	PKGARCH:=all

endef

define Package/luci-app-njitclient/description
	This package contains LuCI configuration pages for njit8021xclient.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-njitclient/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	
	$(INSTALL_CONF) ./files/root/etc/config/njitclient $(1)/etc/config/njitclient
	$(INSTALL_BIN) ./files/root/etc/init.d/njitclient $(1)/etc/init.d/njitclient
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/model/cbi/njitclient.lua $(1)/usr/lib/lua/luci/model/cbi/njitclient.lua
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/controller/njitclient.lua $(1)/usr/lib/lua/luci/controller/njitclient.lua
endef

$(eval $(call BuildPackage,luci-app-njitclient))
