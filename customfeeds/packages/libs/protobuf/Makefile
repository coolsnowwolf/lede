#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=protobuf
PKG_VERSION:=3.15.6
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-cpp-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/google/protobuf/releases/download/v$(PKG_VERSION)
PKG_HASH:=bbdfb7455431d7d58666e8a2996d14b236718ff238eecde10646581e4c87f168

PKG_MAINTAINER:=Ken Keys <kkeys@caida.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:google:protobuf

CMAKE_SOURCE_SUBDIR:=cmake

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/protobuf/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A structured data encoding library
  URL:=https://github.com/google/protobuf
  DEPENDS:=+zlib +libpthread +libatomic +libstdcpp
endef

define Package/protobuf
  $(call Package/protobuf/Default)
  DEPENDS+=+protobuf-lite
endef

define Package/protobuf-lite
  $(call Package/protobuf/Default)
endef

define Package/protobuf/description/Default
Protocol Buffers are a way of encoding structured data in an efficient
yet extensible format. Google uses Protocol Buffers for almost all
of its internal RPC protocols and file formats.
endef

define Package/protobuf/description
$(call Package/protobuf/description/Default)

This package provides the libprotoc, libprotobuf, and libprotobuf-lite
libraries.  For a much smaller protobuf package, see "protobuf-lite".

endef

define Package/protobuf-lite/description
$(call Package/protobuf/description/Default)

This package provides the libprotobuf-lite library.

endef

CMAKE_HOST_OPTIONS += \
	-Dprotobuf_BUILD_PROTOC_BINARIES=ON \
	-Dprotobuf_BUILD_TESTS=OFF \
	-DBUILD_SHARED_LIBS=ON \
	-DCMAKE_SKIP_RPATH=OFF \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_RPATH="${STAGING_DIR_HOSTPKG}/lib"

CMAKE_OPTIONS += \
	-Dprotobuf_BUILD_PROTOC_BINARIES=ON \
	-Dprotobuf_BUILD_TESTS=OFF \
	-Dprotobuf_WITH_ZLIB=ON \
	-DBUILD_SHARED_LIBS=ON

TARGET_LDFLAGS += -latomic $(if $(CONFIG_USE_GLIBC),-lm)

define Build/InstallDev
	$(call Build/InstallDev/cmake,$(1))
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/protobuf.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/protobuf.pc
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/protobuf-lite.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/protobuf-lite.pc
endef

define Package/protobuf-lite/install
	$(INSTALL_DIR) \
		$(1)/usr/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libprotobuf-lite.so* \
		$(1)/usr/lib/
endef

define Package/protobuf/install
	$(INSTALL_DIR) \
		$(1)/usr/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libprotoc.so*  \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libprotobuf.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,protobuf))
$(eval $(call BuildPackage,protobuf-lite))
$(eval $(call HostBuild))
