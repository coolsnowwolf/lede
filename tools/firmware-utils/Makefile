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
PKG_SOURCE_DATE:=2026-09-30
PKG_SOURCE_VERSION:=a1d9589f1ea1b96529ac10fef5d164ff3aa03dc7
PKG_MIRROR_HASH:=4505add3953407dea2be34b541b2b4734fecf25174ca6242dba9e75726dc9fe1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
