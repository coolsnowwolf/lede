include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-xlnetacc
PKG_VERSION:=1.0.5
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for XLNetAcc
	PKGARCH:=all
	DEPENDS:=+jshn +curl +openssl-util +luci-compat
	MAINTAINER:=Sense <sensec@gmail.com>
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/xlnetacc
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/xlnetacc.po $(1)/usr/lib/lua/luci/i18n/xlnetacc.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
