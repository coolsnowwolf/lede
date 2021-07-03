# Copyright (C) 2019 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk


PKG_NAME:=luci-app-qbittorrent
PKG_VERSION=1.0
PKG_RELEASE:=22

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
	choice
		prompt "Build Version Selection"
		default PACKAGE_luci-app-qbittorrent_static if arm||aarch64||x86_64
		default PACKAGE_luci-app-qbittorrent_dynamic if !(arm||aarch64||x86_64)

		config PACKAGE_luci-app-qbittorrent_static
			bool "Static Build"
			depends on arm||aarch64||x86_64

		config PACKAGE_luci-app-qbittorrent_dynamic
			bool "Dynamic Build"
			
	endchoice
endef

LUCI_TITLE:=LuCI support for qBittorrent
LUCI_DEPENDS:=+PACKAGE_luci-app-qbittorrent_static:qBittorrent-static +PACKAGE_luci-app-qbittorrent_dynamic:qbittorrent
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
