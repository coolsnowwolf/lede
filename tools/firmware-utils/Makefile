#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=firmware-utils
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firmware-utils.git
PKG_SOURCE_DATE:=2022-04-25
PKG_SOURCE_VERSION:=1e3d47292b2efbe2665072fa8322f44bd063be39
PKG_MIRROR_HASH:=b1f8e89df4265fa65680a6e10bdd228aaa3dc736b0ee48e12c2debce6ab87c1b

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
