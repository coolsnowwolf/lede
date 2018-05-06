include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dnspod
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/luci-app-dnspod
   SECTION:=luci
   CATEGORY:=LuCI
   SUBMENU:=3. Applications
   TITLE:=Auto DNS for dnspod
   PKGARCH:=all
   DEPENDS:= +curl +libcurl
endef

define Package/luci-app-dnspod/description
   auto get pub ip,dnspod ddns,must include curl with ssl
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-dnspod/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller

	$(INSTALL_CONF) ./files/root/etc/config/dnspod $(1)/etc/config/dnspod
	$(INSTALL_BIN) ./files/root/etc/init.d/dnspod $(1)/etc/init.d/dnspod
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/model/cbi/dnspod.lua $(1)/usr/lib/lua/luci/model/cbi/dnspod.lua
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/controller/dnspod.lua $(1)/usr/lib/lua/luci/controller/dnspod.lua
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/dnspod.sh $(1)/usr/sbin
endef

$(eval $(call BuildPackage,luci-app-dnspod))



