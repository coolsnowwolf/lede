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
PKG_SOURCE_DATE:=2022-07-28
PKG_SOURCE_VERSION:=2483fe782e0d406b65eaa73b39fc27afabd8a9ea
PKG_MIRROR_HASH:=140b84b33cdae1520304cd6798604a51d2f4281b828808a5913d0b2d4b1f551f

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
