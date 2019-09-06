#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=misaka-certificates
PKG_VERSION:=v1.0
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=base
	CATEGORY:=Base system
	TITLE:=Misaka CA certificates
	URL:=https://v2.moegrid.com
endef

define Package/$(PKG_NAME)/description
Misaka CA certificates
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_BIN) ./files/misaka.crt $(1)/etc/ssl/certs/misaka.crt
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
