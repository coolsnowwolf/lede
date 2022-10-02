# 
# Copyright (C) 2013-2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bc
PKG_VERSION:=1.07

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bc
PKG_HASH:=55cf1fc33a728d7c3d386cc7b0cb556eb5bacf8e0cb5a3fcca7f109fc61205ad

PKG_FIXUP := autoreconf
PKG_CPE_ID:=cpe:/a:gnu:bc

include $(INCLUDE_DIR)/host-build.mk

$(eval $(call HostBuild))
