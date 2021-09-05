#
# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-aria2

# Version == major.minor.patch
# increase "minor" on new functionality and "patch" on patches/optimization
PKG_VERSION:=1.0.1

# Release == build
# increase on changes of translation files
PKG_RELEASE:=3

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Hsing-Wang Liao <kuoruan@gmail.com>

# LuCI specific settings
LUCI_TITLE:=LuCI Support for Aria2
LUCI_DEPENDS:=+aria2 +ariang
LUCI_PKGARCH:=all

define Package/$(PKG_NAME)/config
# shown in make menuconfig <Help>
help
		$(LUCI_TITLE)
		.
		Version: $(PKG_VERSION)-$(PKG_RELEASE)
		$(PKG_MAINTAINER)
endef

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature

