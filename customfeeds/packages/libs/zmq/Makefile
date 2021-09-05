#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# This Makefile for ZeroMQ
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zeromq
PKG_VERSION:=4.3.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/zeromq/libzmq/releases/download/v$(PKG_VERSION)
PKG_HASH:=c593001a89f5a85dd2ddf564805deb860e02471171b3f204944857336295c3e5

PKG_MAINTAINER:=Dirk Chang <dirk@kooiot.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENCE.txt
PKG_CPE_ID:=cpe:/a:zeromq:libzmq

CMAKE_BINARY_SUBDIR:=openwrt-build

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libzmq/default
  TITLE:=ZeroMQ - Message Queue engine
  URL:=http://www.zeromq.org/
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libuuid +libpthread +librt +libstdcpp +USE_GLIBC:libbsd
  PROVIDES:=libzmq
endef

define Package/libzmq/default-config
config LIBZMQ_$(2)_WITH_OPENPGM
	depends on PACKAGE_libzmq-$(1)
	bool "enable openpgm support for libzmq-$(1)"
	default n
endef

define Package/libzmq-nc
  $(call Package/libzmq/default)
  VARIANT:=nc
  DEPENDS+=+LIBZMQ_NC_WITH_OPENPGM:openpgm
endef

define Package/libzmq-curve
  $(call Package/libzmq/default)
  VARIANT:=curve
  TITLE+= (CurveZMQ)
  DEPENDS+=+libsodium +LIBZMQ_CURVE_WITH_OPENPGM:openpgm
endef

define Package/libzmq-nc/description
 This package contains the ZeroMQ messaging engine shared library.
endef

define Package/libzmq-curve/description
 $(call Package/libzmq-nc/description)
 Includes CurveZMQ security by libsodium.
endef

define Package/libzmq-nc/config
	$(call Package/libzmq/default-config,nc,NC)
endef

define Package/libzmq-curve/config
	$(call Package/libzmq/default-config,curve,CURVE)
endef

CMAKE_OPTIONS += \
	-DA2X_EXECUTABLE=OFF \
	-DASCIIDOC_EXECUTABLE=OFF \
	-DBUILD_STATIC=OFF \
	-DCMAKE_SKIP_INSTALL_RPATH=ON \
	-DZMQ_HAVE_SOCK_CLOEXEC=ON \
	-DZMQ_HAVE_SO_KEEPALIVE=ON \
	-DZMQ_HAVE_TCP_KEEPCNT=ON \
	-DZMQ_HAVE_TCP_KEEPIDLE=ON \
	-DZMQ_HAVE_TCP_KEEPINTVL=ON \
	-DZMQ_HAVE_TCP_KEEPALIVE=ON \
	-DENABLE_CURVE=ON \
	-DENABLE_EVENTFD=ON \
	-DPOLLER=epoll \
	-DRT_LIBRARY=OFF \
	-DZMQ_BUILD_TESTS=OFF \
	-DWITH_LIBBSD=O$(if $(CONFIG_USE_GLIBC),N,FF)

ifeq ($(BUILD_VARIANT),curve)
	CMAKE_OPTIONS += -DWITH_LIBSODIUM=ON
else
  	CMAKE_OPTIONS += -DWITH_LIBSODIUM=OFF
endif

ifeq ($(CONFIG_LIBZMQ_NC_WITH_OPENPGM),y)
	CMAKE_OPTIONS += -DWITH_OPENPGM=ON
else ifeq ($(CONFIG_LIBZMQ_CURVE_WITH_OPENPGM),y)
	CMAKE_OPTIONS += -DWITH_OPENPGM=ON
else
	CMAKE_OPTIONS += -DWITH_OPENPGM=OFF
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/zmq.h $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/zmq_utils.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzmq.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libzmq.pc $(1)/usr/lib/pkgconfig/
	$(SED) 's,/usr/include,$$$${prefix}/include,g' $(1)/usr/lib/pkgconfig/libzmq.pc
	$(SED) 's,/usr/lib,$$$${exec_prefix}/lib,g' $(1)/usr/lib/pkgconfig/libzmq.pc
endef

define Package/libzmq-nc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzmq.so.* $(1)/usr/lib/
endef

Package/libzmq-curve/install=$(Package/libzmq-nc/install)

$(eval $(call BuildPackage,libzmq-nc))
$(eval $(call BuildPackage,libzmq-curve))
