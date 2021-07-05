#
# Copyright (C) 2006-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=firmware-utils
PKG_RELEASE:=12

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/firmware-utils.git
PKG_SOURCE_DATE:=2021-10-05
PKG_SOURCE_VERSION:=2f70fedc92bd08f9e182fcc388688f36310a597d
PKG_MIRROR_HASH:=c2e63b1f1ef1ef2f8f2d1adb5d030112881500fdadaf82c113b6dabe00cdd06b

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
