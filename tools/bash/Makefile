#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bash
PKG_CPE_ID:=cpe:/a:gnu:bash
PKG_VERSION:=5.2.15

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bash
PKG_HASH:=13720965b5f4fc3a0d4b61dd37e7565c741da9a5be24edc2ae00182fc1b3588c

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
