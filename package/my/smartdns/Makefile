#
# Copyright (C) 2018 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartdns
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
    SECTION:=net
    CATEGORY:=Network
    TITLE:=smartdns
    URL:=
    DEPENDS:=@x86_64
endef

define Package/$(PKG_NAME)/description
  smartdns
endef

define Build/Compile
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) ./files/usr/sbin/smartdns $(1)/usr/sbin/smartdns
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) ./files/etc/config/smartdns $(1)/etc/config/smartdns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) ./files/etc/init.d/smartdns $(1)/etc/init.d/smartdns
	$(INSTALL_DIR) $(1)/etc/smartdns
	$(CP) ./files/etc/smartdns/address.conf $(1)/etc/smartdns/address.conf
	$(CP) ./files/etc/smartdns/smartdns.conf $(1)/etc/smartdns/smartdns.conf
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
