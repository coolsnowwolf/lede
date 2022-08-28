#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bash
PKG_CPE_ID:=cpe:/a:gnu:bash
PKG_VERSION:=5.1.16

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/bash
PKG_HASH:=5bac17218d3911834520dad13cd1f85ab944e1c09ae1aba55906be1f8192f558

HOST_BUILD_PARALLEL := 1

include $(INCLUDE_DIR)/host-build.mk

$(eval $(call HostBuild))
