#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn-easy-rsa

PKG_VERSION:=3.0.4
PKG_RELEASE:=1
PKG_SOURCE_URL:=https://github.com/OpenVPN/easy-rsa/releases/download/v$(PKG_VERSION)/
PKG_SOURCE:=EasyRSA-$(PKG_VERSION).tgz
PKG_HASH:=472167f976c6cb7c860cec6150a5616e163ae20365c81f179811d6ee0779ec5a
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR:=$(BUILD_DIR)/EasyRSA-$(PKG_VERSION)
PKGARCH:=all

include $(INCLUDE_DIR)/package.mk

define Package/openvpn-easy-rsa
  TITLE:=CLI utility to build and manage a PKI CA.
  SECTION:=net
  CATEGORY:=Network
  URL:=http://openvpn.net
  SUBMENU:=VPN
  DEPENDS:=+openssl-util
endef

define Package/openvpn-easy-rsa/conffiles
/etc/easy-rsa/vars
/etc/easy-rsa/openssl-1.0.cnf
/etc/easy-rsa/openssl-easyrsa.cnf
endef

define Build/Configure

endef

define Build/Compile

endef

define Package/openvpn-easy-rsa/install
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/easyrsa $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/easy-rsa
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openssl-easyrsa.cnf $(1)/etc/easy-rsa/openssl-1.0.cnf
	$(LN) /etc/easy-rsa/openssl-1.0.cnf $(1)/etc/easy-rsa/openssl-easyrsa.cnf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/vars.example $(1)/etc/easy-rsa/vars

	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki
	chmod 700 $(1)/etc/easy-rsa/pki
	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki/private
	chmod 700 $(1)/etc/easy-rsa/pki/private
	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki/reqs
	chmod 700 $(1)/etc/easy-rsa/pki/reqs

	$(INSTALL_DIR) $(1)/etc/easy-rsa/x509-types
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/x509-types/* $(1)/etc/easy-rsa/x509-types/

	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) files/openvpn-easy-rsa.upgrade $(1)/lib/upgrade/keep.d/$(PKG_NAME)
endef

$(eval $(call BuildPackage,openvpn-easy-rsa))
