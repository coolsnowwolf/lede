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
PKG_SOURCE_DATE:=2022-09-12
PKG_SOURCE_VERSION:=0c92b20ad488a4fb5fb290f6d1b893df45761275
PKG_MIRROR_HASH:=2cfd36e749b51d539cffe22c77a4c3ccd3bac2eca482affe23e1cab539e8bd16

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
