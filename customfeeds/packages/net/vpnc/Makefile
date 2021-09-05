#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vpnc
PKG_SOURCE_DATE:=2021-01-31
PKG_SOURCE_VERSION:=43780cecd7a61668002f73b6f8b9f9ba61af74ad
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/streambinder/vpnc
PKG_MIRROR_HASH:=df833bbe369bb96cb915da9b63e4dded0f676f06bcdada4ef94e56b8d87b187e

PKG_MAINTAINER:=Daniel Gimpelevich <daniel@gimpelevich.san-francisco.ca.us>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:= \
	CONFIG_VPNC_GNUTLS \
	CONFIG_VPNC_OPENSSL \


include $(INCLUDE_DIR)/package.mk

define Package/vpnc/config
	source "$(SOURCE)/Config.in"
endef

define Package/vpnc
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=\
	  +libgpg-error \
	  +libgcrypt \
	  +kmod-tun \
	  +VPNC_OPENSSL:libopenssl \
	  +VPNC_GNUTLS:libgnutls \
	  +vpnc-scripts \
	  +resolveip \

  TITLE:=VPN client for Cisco EasyVPN
  URL:=https://davidepucci.it/doc/vpnc/
  SUBMENU:=VPN
endef

define Package/vpnc/description
	A VPN client compatible with Cisco's EasyVPN equipment.

	Supports IPSec (ESP) with Mode Configuration and Xauth.  Supports only
	shared-secret IPSec authentication with Xauth, AES (256, 192, 128),
	3DES, 1DES, MD5, SHA1, DH1/2/5 and IP tunneling.
endef

define Package/vpnc/conffiles
/etc/vpnc/default.conf
endef

OPENSSL-y:=OPENSSL_GPL_VIOLATION=yes

define Build/Compile
	mkdir $(PKG_BUILD_DIR)/bin
	$(call Build/Compile/Default, \
		OFLAGS="$(TARGET_CFLAGS)" \
		OS="Linux" \
		VERSION="$(PKG_VERSION)" \
		$(OPENSSL-$(CONFIG_VPNC_OPENSSL)) \
		vpnc \
	)
endef

define Package/vpnc/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/vpnc.sh $(1)/lib/netifd/proto/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP)	$(PKG_BUILD_DIR)/bin/vpnc \
		$(PKG_BUILD_DIR)/src/vpnc-disconnect \
		$(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/vpnc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/src/vpnc.conf $(1)/etc/vpnc/default.conf
	$(INSTALL_DIR) $(1)/lib/upgrade/keep.d
	$(INSTALL_DATA) ./files/vpnc.upgrade $(1)/lib/upgrade/keep.d/vpnc
endef

$(eval $(call BuildPackage,vpnc))
