# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2018 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019-2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-turboacc
PKG_RELEASE:=$(COMMITCOUNT)

PKG_LICENSE:=GPL-3.0-only
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_BBR_CCA \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_DNSFORWARDER \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_DNSPROXY \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_OFFLOADING \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_SHORTCUT_FE
	
LUCI_TITLE:=LuCI support for Flow Offload / Shortcut-FE
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+pdnsd-alt \
	+PACKAGE_$(PKG_NAME)_INCLUDE_BBR_CCA:kmod-tcp-bbr \
	+PACKAGE_$(PKG_NAME)_INCLUDE_DNSFORWARDER:dnsforwarder \
	+PACKAGE_$(PKG_NAME)_INCLUDE_DNSPROXY:dnsproxy \
	+PACKAGE_$(PKG_NAME)_INCLUDE_OFFLOADING:kmod-ipt-offload \
	+PACKAGE_$(PKG_NAME)_INCLUDE_SHORTCUT_FE:kmod-fast-classifier

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_OFFLOADING
	bool "Include Flow Offload"
	depends on PACKAGE_$(PKG_NAME)_INCLUDE_SHORTCUT_FE=n
	default y if i386||x86_64||TARGET_ramips

config PACKAGE_$(PKG_NAME)_INCLUDE_SHORTCUT_FE
	bool "Include Shortcut-FE"
	default y if !(TARGET_ipq806x||TARGET_ipq807x||TARGET_ramips)

config PACKAGE_$(PKG_NAME)_INCLUDE_BBR_CCA
	bool "Include BBR CCA"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_DNSFORWARDER
	bool "Include DNSForwarder"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_DNSPROXY
	bool "Include DNSProxy"
	default n
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
