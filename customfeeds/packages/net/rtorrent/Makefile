#
# Copyright (C) 2007-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtorrent
PKG_VERSION:=0.9.8
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rakshasa/rtorrent/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=bc889ce1dde475ec56aa72ae996912ff58723226a4f4256fef4f1f8636d991d4

PKG_MAINTAINER:=Rosen Penev <rosenp@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=libtorrent

include $(INCLUDE_DIR)/package.mk

define Package/rtorrent/Default
  SUBMENU:=BitTorrent
  SECTION:=net
  CATEGORY:=Network
  TITLE:=BitTorrent client for ncurses
  URL:=https://github.com/rakshasa/rtorrent
  DEPENDS:=+libcurl +libncurses +libpthread +libopenssl +libstdcpp +zlib
endef

define Package/rtorrent/Default/description
 rTorrent is a BitTorrent client for ncurses, using the libtorrent library.
 The client and library is written in C++ with emphasis on speed and
 efficiency, while delivering equivalent features to those found in GUI based
 clients in an ncurses client.
endef

define Package/rtorrent
$(call Package/rtorrent/Default)
  VARIANT:=norpc
endef

define Package/rtorrent/description
$(call Package/rtorrent/Default/description)
 This package is built without xmlrpc support
endef

define Package/rtorrent-rpc
$(call Package/rtorrent/Default)
  VARIANT:=rpc
  DEPENDS+=+xmlrpc-c-server
  TITLE+=(with rpc support)
endef

define Package/rtorrent-rpc/description
$(call Package/rtorrent/Default/description)
 This package is built with xmlrpc support
endef

CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--disable-debug \
	--with-libcurl="$(STAGING_DIR)/usr" \
	--with-ncurses \
	$(if $(CONFIG_IPV6),--enable,--disable)-ipv6

ifeq ($(BUILD_VARIANT),rpc)
CONFIGURE_ARGS += --with-xmlrpc-c
endif

TARGET_CXXFLAGS += -faligned-new
TARGET_LDFLAGS += -lcrypto -lz

define Package/rtorrent/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rtorrent $(1)/usr/bin/
endef

Package/rtorrent-rpc/install = $(Package/rtorrent/install)

$(eval $(call BuildPackage,rtorrent))
$(eval $(call BuildPackage,rtorrent-rpc))
