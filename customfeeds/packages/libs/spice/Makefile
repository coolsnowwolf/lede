#
# Copyright (C) 2019 Yousong Zhou <yszhou4tech@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=spice
PKG_VERSION:=0.15.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://www.spice-space.org/download/releases/spice-server
PKG_HASH:=b320cf8f4bd2852750acb703c15b72856027e5a8554f8217dfbb3cc09deba0f5

PKG_MAINTAINER:=Yousong Zhou <yszhou4tech@gmail.com>
PKG_LICENSE:=LGPL-2.1-only
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=spice-protocol

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include ../../devel/meson/meson.mk

define Package/libspice-server
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SPICE server library
  URL:=https://www.spice-space.org/index.html
  DEPENDS:=+glib2 +libjpeg +libopenssl +pixman +zlib +libstdcpp
endef

# audio codec
MESON_ARGS += \
	-Dopus=disabled

MESON_ARGS += \
	-Dgstreamer=no \
	-Dlz4=false \
	-Dsasl=false \
	-Dsmartcard=disabled \
	-Dalignment-checks=false \
	-Dextra-checks=false \
	-Dstatistics=false \
	-Dmanual=false \
	-Dinstrumentation=no \
	-Dtests=false

MESON_ARGS += \
	-Dspice-common:alignment-checks=false \
	-Dspice-common:extra-checks=false \
	-Dspice-common:opus=disabled \
	-Dspice-common:instrumentation=no \
	-Dspice-common:smartcard=disabled \
	-Dspice-common:python-checks=false \
	-Dspice-common:manual=false \
	-Dspice-common:generate-code=none \
	-Dspice-common:tests=false \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/spice-server
	$(CP) $(PKG_INSTALL_DIR)/usr/include/spice-server/*.h $(1)/usr/include/spice-server
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspice-server.* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/spice-server.pc $(1)/usr/lib/pkgconfig
endef

define Package/libspice-server/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libspice-server.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libspice-server))
