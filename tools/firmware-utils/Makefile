#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=firmware-utils
PKG_RELEASE:=13

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firmware-utils.git
PKG_SOURCE_DATE:=2021-10-05
PKG_SOURCE_VERSION:=f9ad6b335e0fd699398370d4c6e2e45e1c459dea
PKG_MIRROR_HASH:=a1a5c22e8a05b4521e7f6ec20cfa443e9f33a9c07e30718e2f328ee99cebf6cf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
