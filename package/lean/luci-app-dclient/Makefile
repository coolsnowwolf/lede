include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dclient
PKG_VERSION=1.1
PKG_RELEASE:=170914

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI support for dclient
	PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
	This package contains LuCI configuration pages for ZTE 802.1x Client(CCDGUT).
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/src/i18n/zh-cn/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	
	$(INSTALL_CONF) ./src/etc/config/dclient $(1)/etc/config/dclient
	$(INSTALL_BIN) ./src/etc/init.d/dclient $(1)/etc/init.d/dclient
	$(INSTALL_DATA) ./src/luci/model/cbi/dclient.lua $(1)/usr/lib/lua/luci/model/cbi/dclient.lua
	$(INSTALL_DATA) ./src/luci/controller/dclient.lua $(1)/usr/lib/lua/luci/controller/dclient.lua
	$(INSTALL_DATA) ./src/luci/view/dclient_viewer.htm $(1)/usr/lib/lua/luci/view/dclient_viewer.htm
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dclient.*.lmo $(1)/usr/lib/lua/luci/i18n/
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
/etc/init.d/dclient enable
rm -rf /tmp/luci*
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
/etc/init.d/dclient disable
endef

$(eval $(call BuildPackage,luci-app-dclient))
