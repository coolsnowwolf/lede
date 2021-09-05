#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libprotobuf-c
PKG_VERSION:=1.4.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=protobuf-c-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/protobuf-c/protobuf-c/releases/download/v$(PKG_VERSION)
PKG_HASH:=26d98ee9bf18a6eba0d3f855ddec31dbe857667d269bc0b6017335572f85bbcb
PKG_BUILD_DIR:=$(BUILD_DIR)/protobuf-c-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/protobuf-c-$(PKG_VERSION)

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=LICENSE

HOST_BUILD_DEPENDS:=protobuf/host
PKG_BUILD_DEPENDS:=protobuf

CMAKE_INSTALL:=1
CMAKE_SOURCE_SUBDIR:=build-cmake

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libprotobuf-c
  TITLE:=Protocol Buffers library
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=https://github.com/protobuf-c/protobuf-c
endef

define Package/libprotobuf-c/description
  Runtime library to use Google Protocol Buffers from C applications.
  Protocol Buffers are a way of encoding structured data in an efficient yet
  extensible format. Google uses Protocol Buffers for almost all of its
  internal RPC protocols and file formats.
endef

CMAKE_HOST_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DCMAKE_CXX_STANDARD=11 \
	-DCMAKE_SKIP_RPATH=OFF \
	-DCMAKE_INSTALL_RPATH="${STAGING_DIR_HOSTPKG}/lib"

CMAKE_OPTIONS += \
	-DBUILD_SHARED_LIBS=ON \
	-DBUILD_PROTOC=OFF

define Package/libprotobuf-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libprotobuf-c.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libprotobuf-c))
$(eval $(call HostBuild))

