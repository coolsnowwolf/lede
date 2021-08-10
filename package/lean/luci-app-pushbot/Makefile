include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-pushbot
PKG_VERSION:=3.12
PKG_RELEASE:=9

PKG_MAINTAINER:=tty228  zzsj0928

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	DEPENDS:=+iputils-arping +curl
	TITLE:=LuCI support for Pushbot
	PKGARCH:=all
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/pushbot
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
