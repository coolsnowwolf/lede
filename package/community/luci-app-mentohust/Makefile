include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mentohust
PKG_VERSION=1.1.1
PKG_RELEASE:=0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-mentohust
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=MentoHUST 802.1X Client for LuCI
	PKGARCH:=all
endef

define Package/luci-app-mentohust/description
	This package contains LuCI configuration pages for 8021xclient.
endef

define Package/luci-app-mentohust/conffiles
/etc/config/mentohust
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-mentohust/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/mentohust
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/mentohust.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_CONF) ./files/root/etc/config/mentohust $(1)/etc/config/mentohust
	$(INSTALL_BIN) ./files/root/etc/init.d/mentohust $(1)/etc/init.d/mentohust
	$(INSTALL_DATA) ./files/luci/model/cbi/mentohust/general.lua $(1)/usr/lib/lua/luci/model/cbi/mentohust/general.lua
	$(INSTALL_DATA) ./files/luci/model/cbi/mentohust/log.lua $(1)/usr/lib/lua/luci/model/cbi/mentohust/log.lua
	$(INSTALL_DATA) ./files/luci/controller/mentohust.lua $(1)/usr/lib/lua/luci/controller/mentohust.lua
endef

$(eval $(call BuildPackage,luci-app-mentohust))
