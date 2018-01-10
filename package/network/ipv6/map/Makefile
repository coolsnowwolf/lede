#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=map
PKG_VERSION:=4
PKG_RELEASE:=6
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/map
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@IPV6 +kmod-ip6-tunnel +libubox +libubus +iptables-mod-conntrack-extra
  TITLE:=MAP-E and Lightweight 4over6 configuration support
  MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
endef

define Package/map/description
 Provides support for MAP-E (draft-ietf-softwire-map) and
 Lightweight 4over6 (draft-ietf-softwire-lw4over6) in /etc/config/network.
 Refer to http://wiki.openwrt.org/doc/uci/network for
 configuration details.
endef

define Package/map/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/map.sh $(1)/lib/netifd/proto/map.sh
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mapcalc $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,map))
