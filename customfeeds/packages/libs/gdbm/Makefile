#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gdbm
PKG_VERSION:=1.19
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/gdbm
PKG_HASH:=37ed12214122b972e18a0d94995039e57748191939ef74115b1d41d8811364bc

PKG_MAINTAINER:=Marcel Denia <naoir@gmx.net>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/package.mk

define Package/libgdbm
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU database manager
  URL:=https://www.gnu.org/software/gdbm/
endef

define Package/libgdbm/description
  GNU database manager library
  GNU dbm is a set of database routines that use extendible hashing and
  works similar to the standard UNIX dbm routines.
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		BINOWN=`id -u` \
		BINGRP=`id -g` \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/gdbm.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgdbm.{a,so*} $(1)/usr/lib/
endef

define Package/libgdbm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgdbm.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libgdbm))
