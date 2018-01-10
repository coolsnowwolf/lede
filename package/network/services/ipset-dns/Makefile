#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipset-dns
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=http://git.zx2c4.com/ipset-dns
PKG_SOURCE_DATE:=2013-05-03
PKG_SOURCE_VERSION:=6be3afd819a86136b51c5ae722ab48266187155b
PKG_MIRROR_HASH:=4e34943b547babe705c2fad295ee24a5ee0c93ca03e2a9cdeaf323150f97e60b
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk

define Package/ipset-dns/Default
endef

define Package/ipset-dns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A lightweight DNS forwarder to populate ipsets
  URL:=http://git.zx2c4.com/ipset-dns/about/
  DEPENDS:=+libmnl
endef

define Package/ipset-dns/description
 The ipset-dns daemon is a lightweight DNS forwarding server that adds all
 resolved IPs to a given netfilter ipset. It is designed to be used in
 conjunction with dnsmasq's upstream server directive.

 Practical use cases include routing over a given gateway traffic for
 particular web services or webpages that do not have a priori predictable
 IP addresses and instead rely on dizzying arrays of DNS resolutions.
endef

define Package/ipset-dns/conffiles
/etc/config/ipset-dns
endef

define Package/ipset-dns/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipset-dns $(1)/usr/sbin/ipset-dns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ipset-dns.init $(1)/etc/init.d/ipset-dns
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/ipset-dns.config $(1)/etc/config/ipset-dns
endef

$(eval $(call BuildPackage,ipset-dns))
