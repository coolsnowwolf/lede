#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gre
PKG_VERSION:=1
PKG_RELEASE:=7
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/gre/Default
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
endef

define Package/gre
$(call Package/gre/Default)
  TITLE:=Generic Routing Encapsulation config support
endef

define Package/gre/description
 Generic Routing Encapsulation config support (IPv4 and IPv6) in /etc/config/network.
endef

define Package/grev4
$(call Package/gre/Default)
  TITLE:=Generic Routing Encapsulation (IPv4) config support
  DEPENDS:=@(PACKAGE_gre) +kmod-gre +resolveip
endef

define Package/grev4/description
 Generic Routing Encapsulation config support (IPv4) in /etc/config/network.
endef

define Package/grev6
$(call Package/gre/Default)
  TITLE:=Generic Routing Encapsulation (IPv6) config support
  DEPENDS:=@(PACKAGE_gre) @IPV6 +kmod-gre6 +resolveip
endef

define Package/grev6/description
 Generic Routing Encapsulation config support (IPv6) in /etc/config/network.
endef

define Build/Compile
endef

define Build/Configure
endef

define Package/gre/install
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./files/gre.sh $(1)/lib/netifd/proto/gre.sh
endef

define Package/grev4/install
	:
endef

define Package/grev6/install
	:
endef

$(eval $(call BuildPackage,gre))
$(eval $(call BuildPackage,grev4))
$(eval $(call BuildPackage,grev6))
