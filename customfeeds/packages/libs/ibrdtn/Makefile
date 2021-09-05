#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ibrdtn
PKG_VERSION:=1.0.1
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
PKG_HASH:=288b14ccbaefb5e3234065c2778c247797ccb3c7afbb6746bb37dc12c620d360
PKG_MAINTAINER:=Johannes Morgenroth <jm@m-network.de>
PKG_LICENSE:=Apache-2.0

PKG_INSTALL:=1
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

define Package/ibrdtn
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+ibrcommon +zlib
  TITLE:=IBR-DTN Library
endef

define Package/ibrdtn/description
 Base library for IBR-DTN daemon and tools.
endef

CONFIGURE_ARGS += \
	--with-compression \
	--without-glib

TARGET_CXXFLAGS +=-std=c++11

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/ibrdtn/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,ibrdtn))
