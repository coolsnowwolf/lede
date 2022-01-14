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
PKG_SOURCE_DATE:=2022-01-14
PKG_SOURCE_VERSION:=84dbf8ee49f522d3a4528763c9473cf3dd7c8c52
PKG_MIRROR_HASH:=aabf70dbc155d10175ee3b25f375953bef31bf7e25a2391fda8687aa93ea79e6

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
