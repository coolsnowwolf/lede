#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ibrcommon
PKG_VERSION:=1.0.1
PKG_RELEASE:=9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.ibr.cs.tu-bs.de/projects/ibr-dtn/releases
PKG_HASH:=9c457c1ebc01e6216524636628c647bef34ab11bd96f0e0788be8749374fdc20
PKG_MAINTAINER:=Johannes Morgenroth <jm@m-network.de>
PKG_LICENSE:=Apache-2.0

PKG_INSTALL:=1
PKG_FIXUP:=libtool

include $(INCLUDE_DIR)/package.mk

define Package/ibrcommon
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +librt +libnl +libopenssl
  TITLE:=IBR Common C++ Library
endef

CONFIGURE_ARGS += \
	--with-openssl

TARGET_CXXFLAGS += -std=c++11

define Package/ibrcommon/description
 A library with common functions for C++.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/ibrcommon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,ibrcommon))
