#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lzo
PKG_VERSION:=2.10
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.oberhumer.com/opensource/lzo/download/
PKG_HASH:=c0f892943208266f9b6543b3ae308fab6284c5c90e627931446fb49b4221a072

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:lzo_project:lzo

CMAKE_BINARY_SUBDIR:=openwrt-build

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_HOST_OPTIONS += \
	-DENABLE_SHARED=ON \
	-DENABLE_TESTS=OFF \
	-DENABLE_EXAMPLES=OFF \
	-DENABLE_DOCS=ON \
	-DBUILD_TESTING=OFF

define Host/Uninstall
	rm -rf $(HOST_BUILD_PREFIX)/include/lzo
	rm -f  $(HOST_BUILD_PREFIX)/lib/liblzo2.a
	rm -f  $(HOST_BUILD_PREFIX)/lib/liblzo2.so.2.0.0
	rm -f  $(HOST_BUILD_PREFIX)/lib/liblzo2.so.2
	rm -f  $(HOST_BUILD_PREFIX)/lib/liblzo2.so
	rm -f  $(HOST_BUILD_PREFIX)/lib/pkgconfig/lzo2.pc
endef

$(eval $(call HostBuild))
