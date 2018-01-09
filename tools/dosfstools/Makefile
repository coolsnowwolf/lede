#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016 LEDE-Project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=dosfstools
PKG_CPE_ID:=cpe:/a:dosfstools_project:dosfstools
PKG_VERSION:=4.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://github.com/dosfstools/dosfstools/releases/download/v$(PKG_VERSION)/ \
		http://fossies.org/linux/misc
PKG_HASH:=e6b2aca70ccc3fe3687365009dd94a2e18e82b688ed4e260e04b7412471cc173

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

ifeq ($(HOST_OS),Darwin)
HOST_CFLAGS += -UHAVE_ENDIAN_H
endif

$(eval $(call HostBuild))
