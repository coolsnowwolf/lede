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
PKG_SOURCE_DATE:=2023-01-29
PKG_SOURCE_VERSION:=e7233d229c2c204563a8c70231b34ebb2a197400
PKG_MIRROR_HASH:=06ef2351187c3c4ee164099c1f50f83dfb5f56ed2aec0db8e1db20efb2c157d5

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
