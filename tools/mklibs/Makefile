#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mklibs
PKG_VERSION:=0.1.45

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@DEBIAN/pool/main/m/mklibs/
PKG_HASH:=dd92a904b3942566f713fe536cd77dd1a5cfc62243c0e0bc6bb5d866e37422f3

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS += -I$(CURDIR)/include
HOST_CPPFLAGS += -std=gnu++98

define Host/Install
       $(INSTALL_BIN) \
               $(HOST_BUILD_DIR)/src/mklibs \
               $(HOST_BUILD_DIR)/src/mklibs-copy \
               $(HOST_BUILD_DIR)/src/mklibs-readelf/mklibs-readelf \
               $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
       rm -f $(STAGING_DIR_HOST)/bin/mklibs*
endef

$(eval $(call HostBuild))
