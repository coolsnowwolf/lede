#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dhcpcd
PKG_VERSION:=7.2.2
PKG_RELEASE:=1

PKG_SOURCE_URL:=ftp://roy.marples.name/pub/dhcpcd \
    http://roy.marples.name/downloads/dhcpcd
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=3db7ff18cba9274da1d2176fb3c7cbe23926a8e58d5c8e244ad55c62d38ba09e

PKG_LICENSE:=BSD-2c
PKG_LICENSE_FILES:=
PKG_CPE_ID:=cpe:/a:roy_marples:dhcpcd

PKG_MAINTAINER:=Marko Ratkaj <marko.ratkaj@sartura.hr>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dhcpcd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=DHCPv4/IPv4LL/IPv6RS/DHCPv6 quad stack client
  URL:=http://roy.marples.name/projects/dhcpcd
endef

define Package/dhcpcd/description
  DHCPv4, IPv6RS and DHCPv6 client with IPv4LL support
  dhcpcd is a one stop network management daemon which includes
    * RFC compliant DHCPv4 and DHCPv6 clients
    * DHCPv6 Prefix Delegation support
    * IPv4LL (aka ZeroConf) support
    * ARP address conflict resolution
    * Link carrier detection
    * Wireless SSID profiles
    * ARP ping profiles
endef

CONFIGURE_ARGS+= \
	--prefix=/ \
	--sbindir=/sbin \
	--libexecdir=/lib/dhcpcd \
	--dbdir=/var/dhcpcd

define Package/dhcpcd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc $(1)/lib/dhcpcd/dhcpcd-hooks
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/dhcpcd $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/lib/dhcpcd/dhcpcd-run-hooks \
	    $(1)/lib/dhcpcd/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/lib/dhcpcd/dhcpcd-hooks/* \
	    $(1)/lib/dhcpcd/dhcpcd-hooks/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dhcpcd.init $(1)/etc/init.d/dhcpcd
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/dhcpcd.conf $(1)/etc/dhcpcd.conf
endef

$(eval $(call BuildPackage,dhcpcd))
