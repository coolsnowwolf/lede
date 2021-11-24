# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2019 ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-rclone
PKG_VERSION:=1.4.1
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3.0+
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng

LUCI_TITLE:=LuCI support for Rclone
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+rclone-config \
	+PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui:rclone-webui-react \
	+PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng:rclone-ng

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_rclone-webui
	bool "Include rclone-webui"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_rclone-ng
	bool "Include rclone-ng (another webui)"
	default y
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
