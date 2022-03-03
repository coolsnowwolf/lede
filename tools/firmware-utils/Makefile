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
PKG_SOURCE_DATE:=2022-02-28
PKG_SOURCE_VERSION:=002cfaf0204ef48fdced6f43c99d59394860e2cb
PKG_MIRROR_HASH:=aa2ab50ab24474cd8b8342d8c6d88309a13117e2f1a34b7bfc61b15e0510bce6

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
