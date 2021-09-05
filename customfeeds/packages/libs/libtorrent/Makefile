#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libtorrent
PKG_VERSION:=0.13.8
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rakshasa/libtorrent/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=0f6c2e7ffd3a1723ab47fdac785ec40f85c0a5b5a42c1d002272205b988be722

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libtorrent
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Rakshasa's BitTorrent library
  URL:=https://rakshasa.github.io/rtorrent/
  DEPENDS:=+libopenssl +libstdcpp +zlib
  BUILDONLY:=1
endef

define Package/libtorrent/description
 LibTorrent is a BitTorrent library written in C++ for *nix, with a focus on
 high performance and good code. The library differentiates itself from other
 implementations by transferring directly from file pages to the network stack.
 On high-bandwidth connections it is able to seed at 3 times the speed of the
 official client.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-aligned \
	--enable-openssl \
	--disable-debug \
	--disable-instrumentation \
	--with-epoll \
	--with-zlib=$(STAGING_DIR)/usr \
	--without-kqueue

TARGET_CXXFLAGS += -faligned-new

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/torrent $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtorrent.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libtorrent.pc $(1)/usr/lib/pkgconfig/
endef

$(eval $(call BuildPackage,libtorrent))
