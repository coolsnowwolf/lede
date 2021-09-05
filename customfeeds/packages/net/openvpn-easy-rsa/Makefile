#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn-easy-rsa

PKG_VERSION:=3.0.8
PKG_RELEASE:=3
PKG_SOURCE_URL:=https://codeload.github.com/OpenVPN/easy-rsa/tar.gz/v$(PKG_VERSION)?
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=fd6b67d867c3b8afd53efa2ca015477f6658a02323e1799432083472ac0dd200

# For git snapshots
#PKG_SOURCE_PROTO:=git
#PKG_RELEASE=0git$(PKG_SOURCE_DATE)
#PKG_SOURCE_URL:=https://github.com/OpenVPN/easy-rsa.git
#PKG_SOURCE_DATE:=2020-03-30
#PKG_SOURCE_VERSION:=945c9359f6ae3796df21e2986e49489718e0d5f8
#PKG_MIRROR_HASH:=

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_BUILD_DIR:=$(BUILD_DIR)/easy-rsa-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/openvpn-easy-rsa
  TITLE:=CLI utility to build and manage a PKI CA.
  SECTION:=net
  CATEGORY:=Network
  URL:=http://openvpn.net
  SUBMENU:=VPN
  DEPENDS:=+openssl-util
  PKGARCH:=all
endef

define Package/openvpn-easy-rsa/conffiles
/etc/easy-rsa/vars
/etc/easy-rsa/openssl-1.0.cnf
/etc/easy-rsa/openssl-easyrsa.cnf
/etc/profile.d/50-$(PKG_NAME).sh
endef

define Build/Configure
endef

define Build/Compile
	cd $(PKG_BUILD_DIR); \
	$(PKG_BUILD_DIR)/build/build-dist.sh \
		--no-windows \
		--no-compress \
		--dist-clean \
		--version=$(PKG_VERSION)
endef

define Package/openvpn-easy-rsa/install

	$(INSTALL_DIR) $(1)/usr/lib/easy-rsa/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dist-staging/unix/EasyRSA-$(PKG_VERSION)/easyrsa $(1)/usr/lib/easy-rsa/

	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../lib/easy-rsa/easyrsa $(1)/usr/bin/easyrsa

	$(INSTALL_DIR) $(1)/etc/easy-rsa
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist-staging/unix/EasyRSA-$(PKG_VERSION)/openssl-easyrsa.cnf $(1)/etc/easy-rsa/openssl-1.0.cnf
	$(LN) openssl-1.0.cnf $(1)/etc/easy-rsa/openssl-easyrsa.cnf
	$(LN) ../../../etc/easy-rsa/openssl-easyrsa.cnf $(1)/usr/lib/easy-rsa/openssl-easyrsa.cnf
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist-staging/unix/EasyRSA-$(PKG_VERSION)/vars.example $(1)/etc/easy-rsa/vars
	$(LN) ../../../etc/easy-rsa/vars $(1)/usr/lib/easy-rsa/vars

	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki
	chmod 700 $(1)/etc/easy-rsa/pki
	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki/private
	chmod 700 $(1)/etc/easy-rsa/pki/private
	$(INSTALL_DIR) $(1)/etc/easy-rsa/pki/reqs
	chmod 700 $(1)/etc/easy-rsa/pki/reqs

	$(INSTALL_DIR) $(1)/etc/easy-rsa/x509-types
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist-staging/unix/EasyRSA-$(PKG_VERSION)/x509-types/* $(1)/etc/easy-rsa/x509-types/
	$(LN) ../../../etc/easy-rsa/x509-types $(1)/usr/lib/easy-rsa/x509-types

	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) files/openvpn-easy-rsa.upgrade $(1)/lib/upgrade/keep.d/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/etc/profile.d
	$(INSTALL_DATA) files/openvpn-easy-rsa.profile $(1)/etc/profile.d/50-$(PKG_NAME).sh
endef

$(eval $(call BuildPackage,openvpn-easy-rsa))
