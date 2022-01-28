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
PKG_SOURCE_DATE:=2022-01-28
PKG_SOURCE_VERSION:=8e7274e02fdc6f2cb61b415d6e5b2e1c7e977aa1
PKG_MIRROR_HASH:=bf3e3f9e37b156107d7bc2ce666112db2b3ef6580b1f47bce96311da312fe169

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
