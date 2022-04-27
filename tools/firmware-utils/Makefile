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
PKG_SOURCE_VERSION:=ddc3e00e314d3fbc3f9faab2d07395722ce9b01a
PKG_MIRROR_HASH:=246fc1d72d3a8cdb4072d81e033c92abaf614acd6f35a10fffd029d5c7f9303b

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
