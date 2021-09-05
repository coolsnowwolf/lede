#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=igmpproxy
PKG_VERSION:=0.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/pali/igmpproxy/releases/download/${PKG_VERSION}/
PKG_HASH:=d1fc244cb2fbbf99f720bda3e841fe59ece9b6919073790b4b892739b1b844eb
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>

include $(INCLUDE_DIR)/package.mk

PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0+

define Package/igmpproxy
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Routing and Redirection
  DEPENDS:=+USE_GLIBC:librt
  TITLE:=Multicast Routing Daemon
  URL:=http://sourceforge.net/projects/igmpproxy
endef

define Package/igmpproxy/description
	IGMPproxy is a simple dynamic Multicast Routing Daemon using
	only IGMP signalling (Internet Group Management Protocol).
endef

define Package/igmpproxy/conffiles
/etc/config/igmpproxy
endef

define Package/igmpproxy/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/igmpproxy.config $(1)/etc/config/igmpproxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/igmpproxy.init $(1)/etc/init.d/igmpproxy
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/igmpproxy $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,igmpproxy))
