#
# Copyright (C) 2016 Toke Høiland-Jørgensen
#
# This is free software, licensed under the GNU General Public License v3 or
# later.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=acme
PKG_VERSION:=2.9.0
PKG_RELEASE:=6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/acmesh-official/acme.sh/tar.gz/$(PKG_VERSION)?
PKG_HASH:=bb8cfb8333bc9e986bb8b7644d518bc238b353c871c343b7edf8cda7b7dd903c
PKG_BUILD_DIR:=$(BUILD_DIR)/acme.sh-$(PKG_VERSION)

PKG_MAINTAINER:=Toke Høiland-Jørgensen <toke@toke.dk>
PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE.md

include $(INCLUDE_DIR)/package.mk

define Package/acme
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+wget-ssl +ca-bundle +openssl-util +socat
  TITLE:=ACME (Letsencrypt) client
  URL:=https://acme.sh
  PKGARCH:=all
endef

define Package/acme/description
 A client for issuing Letsencrypt certificates.
endef

define Package/acme/conffiles
/etc/config/acme
/etc/acme
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/acme/install
	$(INSTALL_DIR) $(1)/etc/acme
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/acme.config $(1)/etc/config/acme
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/acme.init $(1)/etc/init.d/acme
	$(INSTALL_DIR) $(1)/usr/lib/acme
	$(INSTALL_BIN) ./files/run.sh $(1)/usr/lib/acme/run-acme
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/acme.sh $(1)/usr/lib/acme/acme.sh
endef

define Package/acme/prerm
#!/bin/sh
sed -i '/\/etc\/init\.d\/acme start/d' /etc/crontabs/root
endef

define Package/acme-dnsapi
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+acme
  TITLE:=DNS API integration for ACME (Letsencrypt) client
  PKGARCH:=all
endef

define Package/acme-dnsapi/description
 This package provides DNS API integration for ACME (Letsencrypt) client.
endef

define Package/acme-dnsapi/install
	$(INSTALL_DIR) $(1)/usr/lib/acme/dnsapi
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dnsapi/*.sh $(1)/usr/lib/acme/dnsapi
endef

$(eval $(call BuildPackage,acme))
$(eval $(call BuildPackage,acme-dnsapi))
