# Copyright (C) 2017 Yousong Zhou

include $(TOPDIR)/rules.mk

PKG_NAME:=pservice
PKG_VERSION:=2017-08-29
PKG_RELEASE=3

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/pservice
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Wrap commands as procd services
endef

define Build/Compile
endef

define Package/pservice/conffiles
/etc/config/pservice
endef

define Package/pservice/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./files/pservice.init $(1)/etc/init.d/pservice
	$(INSTALL_DATA) ./files/pservice.config $(1)/etc/config/pservice
endef

$(eval $(call BuildPackage,pservice))
