include $(TOPDIR)/rules.mk

PKG_NAME:=sysuh3c
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/haswelliris/h3c.git
PKG_SOURCE_VERSION:=fdf66c3

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=H3C client
endef

define Build/Configure
	$(call Build/Configure/Default)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/h3c $(1)/usr/sbin/sysuh3c
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/sysuh3c.config $(1)/etc/config/sysuh3c
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/sysuh3c.init $(1)/etc/init.d/sysuh3c
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
