#
# Copyright 2008 Steven Barth <steven@midlink.org>
# Copyright 2008 Jo-Philipp Wich <jow@openwrt.org>
# Copyright 2013 Manuel Munz <freifunk at somakoma dot de>
# Copyright 2014-2018 Christian Schoenebeck <christian dot schoenebeck at gmail dot com>
#
# This is free software, licensed under the Apache License, Version 2.0

include $(TOPDIR)/rules.mk

# PKG_NAME:=luci-app-ddns

# Version == major.minor.patch
# increase on new functionality (minor) or patches (patch)
PKG_VERSION:=2.4.9

# Release == build
# increase on changes of translation files
PKG_RELEASE:=8

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Ansuel Smith <ansuelsmth@gmail.com>

# LuCI specific settings
LUCI_TITLE:=LuCI Support for Dynamic DNS Client (ddns-scripts)
LUCI_DEPENDS:=+luci-mod-admin-full +ddns-scripts
# LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/config
# shown in make menuconfig <Help>
help
	$(LUCI_TITLE)
	Version: $(PKG_VERSION)-$(PKG_RELEASE)
endef

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
