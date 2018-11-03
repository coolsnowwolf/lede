#
# Copyright (C) 2010-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn

PKG_VERSION:=2.4.5
PKG_RELEASE:=7

PKG_SOURCE_URL:=\
	https://build.openvpn.net/downloads/releases/ \
	https://swupdate.openvpn.net/community/releases/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=43c0a363a332350f620d1cd93bb431e082bedbc93d4fb872f758650d53c1d29e

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:openvpn:openvpn

include $(INCLUDE_DIR)/package.mk

define Package/openvpn/Default
  TITLE:=Open source VPN solution using $(2)
  SECTION:=net
  CATEGORY:=Network
  URL:=http://openvpn.net
  SUBMENU:=VPN
  MENU:=1
  DEPENDS:=+kmod-tun +OPENVPN_$(1)_ENABLE_LZO:liblzo +OPENVPN_$(1)_ENABLE_IPROUTE2:ip $(3)
  VARIANT:=$(1)
ifeq ($(1),nossl)
  PROVIDES:=openvpn
else
  PROVIDES:=openvpn openvpn-crypto
endif
endef

Package/openvpn-openssl=$(call Package/openvpn/Default,openssl,OpenSSL,+PACKAGE_openvpn-openssl:libopenssl)
Package/openvpn-mbedtls=$(call Package/openvpn/Default,mbedtls,mbedTLS,+PACKAGE_openvpn-mbedtls:libmbedtls)
Package/openvpn-nossl=$(call Package/openvpn/Default,nossl,plaintext (no SSL))

define Package/openvpn/config/Default
	source "$(SOURCE)/Config-$(1).in"
endef

Package/openvpn-openssl/config=$(call Package/openvpn/config/Default,openssl)
Package/openvpn-mbedtls/config=$(call Package/openvpn/config/Default,mbedtls)
Package/openvpn-nossl/config=$(call Package/openvpn/config/Default,nossl)

ifeq ($(BUILD_VARIANT),mbedtls)
CONFIG_OPENVPN_MBEDTLS:=y
endif
ifeq ($(BUILD_VARIANT),openssl)
CONFIG_OPENVPN_OPENSSL:=y
endif
ifeq ($(BUILD_VARIANT),nossl)
CONFIG_OPENVPN_NOSSL:=y
endif

CONFIGURE_VARS += \
	IFCONFIG=/sbin/ifconfig \
	ROUTE=/sbin/route \
	IPROUTE=/sbin/ip \
	NETSTAT=/sbin/netstat

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

define Build/Configure
	$(call Build/Configure/Default, \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SMALL),--enable-small) \
		--disable-selinux \
		--disable-systemd \
		--disable-plugins \
		--disable-debug \
		--disable-pkcs11 \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_LZO),--enable,--disable)-lzo \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_LZ4),--enable,--disable)-lz4 \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_X509_ALT_USERNAME),enable,disable-x509-alt-username)-ssl \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SERVER),--enable,--disable)-server \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_MANAGEMENT),--enable,--disable)-management \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SOCKS),--enable,--disable)-socks \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_HTTP),--enable,--disable)-http-proxy \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_FRAGMENT),--enable,--disable)-fragment \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_MULTIHOME),--enable,--disable)-multihome \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_IPROUTE2),--enable,--disable)-iproute2 \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_DEF_AUTH),--enable,--disable)-def-auth \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_PF),--enable,--disable)-pf \
		$(if $(CONFIG_OPENVPN_NOSSL),--disable-ssl --disable-crypto,--enable-ssl --enable-crypto) \
		$(if $(CONFIG_OPENVPN_OPENSSL),--with-crypto-library=openssl) \
		$(if $(CONFIG_OPENVPN_MBEDTLS),--with-crypto-library=mbedtls) \
	)
endef

define Package/openvpn-$(BUILD_VARIANT)/conffiles
/etc/config/openvpn
endef

define Package/openvpn-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) \
		$(1)/usr/sbin \
		$(1)/usr/share/openvpn \
		$(1)/etc/init.d \
		$(1)/etc/config \
		$(1)/etc/openvpn \
		$(1)/lib/upgrade/keep.d

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/openvpn \
		$(1)/usr/sbin/

	$(INSTALL_BIN) \
		files/openvpn.init \
		$(1)/etc/init.d/openvpn
	$(INSTALL_DATA) \
		files/openvpn.options \
		$(1)/usr/share/openvpn/openvpn.options

	$(INSTALL_CONF) files/openvpn.config \
		$(1)/etc/config/openvpn

	$(INSTALL_DATA) \
		files/openvpn.upgrade \
		$(1)/lib/upgrade/keep.d/openvpn
endef

$(eval $(call BuildPackage,openvpn-openssl))
$(eval $(call BuildPackage,openvpn-mbedtls))
$(eval $(call BuildPackage,openvpn-nossl))
