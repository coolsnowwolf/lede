#
# Copyright (C) 2017 Lucian Cristian <lucian.cristian@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
# updated to work with latest source from abrasive
#

include $(TOPDIR)/rules.mk

PKG_NAME:=musl-fts
PKG_VERSION:=1.2.7
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/pullmoll/musl-fts.git
PKG_SOURCE_VERSION:=0bde52df588e8969879a2cae51c3a4774ec62472
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_MIRROR_HASH:=29c62a600128e9189b1b2e1aea568546178eedf739527f657873b3b773072ecb

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>

PKG_LICENSE:=LGPL-2.1
PKG_LICENSE_FILES:=COPYING AUTHORS

PKG_FIXUP:=autoreconf
PKG_REMOVE_FILES:=autogen.sh

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/musl-fts
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=fts implementation for musl libc
  URL:=https://github.com/pullmoll/musl-fts
  DEPENDS:= +libpthread
endef

define Package/musl-fts/description
  The musl-fts package implements the fts(3) functions fts_open, fts_read, fts_children, fts_set and fts_close, which are missing in musl libc.
endef

HOST_CONFIGURE_ARGS += --disable-shared --with-pic

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/fts.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfts.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/musl-fts.pc $(1)/usr/lib/pkgconfig/
endef

define Package/musl-fts/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libfts.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,musl-fts))
$(eval $(call HostBuild))
