#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=firewall
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firewall3.git
PKG_SOURCE_DATE:=2021-03-29
PKG_SOURCE_VERSION:=a4355a6faea2c4d59c90cd52851fa9aab4ccee5c
PKG_MIRROR_HASH:=747db79b11010bbfb708aae1793857c8d959d9270c7392f8c78d2d2110cf2fe7
PKG_MAINTAINER:=Jo-Philipp Wich <jo@mein.io>
PKG_LICENSE:=ISC

PKG_CONFIG_DEPENDS := CONFIG_IPV6

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/firewall
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=OpenWrt C Firewall
  DEPENDS:=+libubox +libubus +libuci +libip4tc +IPV6:libip6tc +libxtables +kmod-ipt-core +kmod-ipt-conntrack +IPV6:kmod-nf-conntrack6 +kmod-ipt-nat +iptables-mod-fullconenat
endef

define Package/firewall/description
 This package provides a config-compatible C implementation of the UCI firewall.
endef

define Package/firewall/conffiles
/etc/config/firewall
/etc/firewall.user
endef

TARGET_CFLAGS += -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -flto
CMAKE_OPTIONS += $(if $(CONFIG_IPV6),,-DDISABLE_IPV6=1)

define Package/firewall/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/firewall3 $(1)/sbin/fw3
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/firewall.init $(1)/etc/init.d/firewall
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_CONF) ./files/firewall.hotplug $(1)/etc/hotplug.d/iface/20-firewall
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_CONF) ./files/firewall.config $(1)/etc/config/firewall
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_CONF) ./files/firewall.user $(1)/etc/firewall.user
	$(INSTALL_DIR) $(1)/usr/share/fw3
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/helpers.conf $(1)/usr/share/fw3
endef

$(eval $(call BuildPackage,firewall))
