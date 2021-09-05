#
# Copyright (C) 2016-2017 Jo-Philipp Wich <jo@mein.io>
#
# Licensed under the Apache License, Version 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcd-mod-rrdns
PKG_VERSION:=20170710
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_LICENSE:=Apache-2.0

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/rpcd-mod-rrdns
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Rapid reverse DNS rpcd module
  DEPENDS:=+rpcd +libubox +libubus
endef

define Package/rpcd-mod-rrdns/description
 Provides rapid mass reverse DNS lookup functionality.
endef

define Package/rpcd-mod-rrdns/install
	$(INSTALL_DIR) $(1)/usr/lib/rpcd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rrdns.so $(1)/usr/lib/rpcd/
endef

define Package/rpcd-mod-rrdns/postinst
#!/bin/sh
killall -HUP rpcd 2>/dev/null
exit 0
endef

$(eval $(call BuildPackage,rpcd-mod-rrdns))
