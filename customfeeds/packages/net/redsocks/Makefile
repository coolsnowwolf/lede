#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=redsocks
PKG_VERSION:=0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-release-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/darkk/redsocks/tar.gz/release-$(PKG_VERSION)?
PKG_HASH:=bbeb531d7f1986d7102f1bd6733dacce41d3f3ba7d604f1aab61c63e0ba2ee62
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-release-$(PKG_VERSION)

PKG_MAINTAINER:=Johannes Morgenroth <jm@m-network.de>
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/redsocks
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libevent2-core
  TITLE:=Redirect any TCP connection to a SOCKS or HTTPS proxy server
endef

define Package/redsocks/conffiles
/etc/redsocks.conf
endef

define Package/redsocks/description
 Redsocks is a daemon running on the local system, that will transparently
 tunnel any TCP connection via a remote SOCKS4, SOCKS5 or HTTP proxy server. It
 uses the system firewall's redirection facility to intercept TCP connections,
 thus the redirection is system-wide, with fine-grained control, and does
 not depend on LD_PRELOAD libraries.

 Redsocks supports tunneling TCP connections and UDP packets. It has
 authentication support for both, SOCKS and HTTP proxies.

 Also included is a small DNS server returning answers with the "truncated" flag
 set for any UDP query, forcing the resolver to use TCP.
endef

define Package/redsocks/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/redsocks $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) files/redsocks.init $(1)/etc/init.d/redsocks
	$(INSTALL_CONF) files/redsocks.conf $(1)/etc/
endef

$(eval $(call BuildPackage,redsocks))
