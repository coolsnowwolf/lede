#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mklibs
PKG_VERSION:=0.1.35

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ftp.de.debian.org/debian/pool/main/m/mklibs/
PKG_HASH:=ccb1023dc1729c5a37ca6c3eca8e4bac3491116763c8820dfce8eea4845c8567

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS += -I$(CURDIR)/include

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
