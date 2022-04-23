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
PKG_SOURCE_DATE:=2022-04-23
PKG_SOURCE_VERSION:=05fd7007b0c842cb560b83f07b50282733ccf2ca
PKG_MIRROR_HASH:=58a0578cd1a8bf934dc683fc3cced6208b17f528fe2f0329f169ea50eb83bc76

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

$(eval $(call HostBuild))
