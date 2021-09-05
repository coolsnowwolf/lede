#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aggregate
PKG_VERSION:=1.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://ftp.isc.org/isc/aggregate/
PKG_HASH:=166503005cd8722c730e530cc90652ddfa198a25624914c65dffc3eb87ba5482

PKG_MAINTAINER:=Nikil Mehta <nikil.mehta@gmail.com>
PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/aggregate
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=IP Addresses and Names
  TITLE:=ipv4 cidr prefix aggregator
  URL:=https://ftp.isc.org/isc/aggregate/
endef

define Package/aggregate/description
	takes a list of prefixes in conventional format on stdin, and performs two
	optimisations to reduce the length of the prefix list. It removes any
	supplied prefixes which are supurfluous because they are already included in
	another supplied prefix (e.g., 203.97.2.0/24 would be removed if
	203.97.0.0/17 was also supplied), and identifies adjacent prefixes that can
	be combined under a single, shorter-length prefix (e.g., 203.97.2.0/24 and
	203.97.3.0/24 can be combined into the single prefix 203.97.2.0/23).
endef

define Package/aggregate/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/aggregate $(1)/usr/bin/
endef

$(eval $(call BuildPackage,aggregate))
