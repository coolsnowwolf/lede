# Copyright (C) 2020 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-unblockmusic
PKG_VERSION:=2.3.5
PKG_RELEASE:=18

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=lean

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_Go \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_NodeJS

LUCI_TITLE:=LuCI support for Unblock NeteaseCloudMusic
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+dnsmasq-full +ipset +uclient-fetch \
	+PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_Go:UnblockNeteaseMusic-Go \
	+PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_NodeJS:UnblockNeteaseMusic

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_Go
	bool "UnblockNeteaseMusic Golang Version"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_UnblockNeteaseMusic_NodeJS
	bool "UnblockNeteaseMusic NodeJS Version"
	depends on HAS_FPU || KERNEL_MIPS_FPU_EMULATOR || SOFT_FLOAT
	default y if i386||x86_64||arm||aarch64
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/unblockmusic
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
