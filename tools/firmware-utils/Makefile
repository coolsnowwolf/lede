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
PKG_SOURCE_DATE:=2022-12-15
PKG_SOURCE_VERSION:=bd856eff48509fa4511169ee55f639ede8dd4ac6
PKG_MIRROR_HASH:=5765389fdb19e0ea2d8f1f586d60580475f6a9917a46777200a3a011f5f654fa

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
