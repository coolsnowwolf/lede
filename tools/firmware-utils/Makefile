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
PKG_SOURCE_DATE:=2024-10-20
PKG_SOURCE_VERSION:=4b7638925d3eac03e614e40bc30cb49f5877c46d
PKG_MIRROR_HASH:=849193bcd9b7be96eada2219f3e8b2d229186cae4e4b7a8746375480fbcd2683

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
