#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lzop
PKG_VERSION:=1.04

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.lzop.org/download/
PKG_HASH:=7e72b62a8a60aff5200a047eea0773a8fb205caf7acbe1774d95147f305a2f41

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

CMAKE_BINARY_SUBDIR:=openwrt-build

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_HOST_OPTIONS += \
	-DENABLE_DOCS=ON \
	-DBUILD_TESTING=OFF

define Host/Uninstall
	rm -f $(STAGING_DIR_HOST)/bin/lzop
endef

$(eval $(call HostBuild))
