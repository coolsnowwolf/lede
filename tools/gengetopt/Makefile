#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licengengetopt under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gengetopt
PKG_VERSION:=2.22.6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=30b05a88604d71ef2a42a2ef26cd26df242b41f5b011ad03083143a31d9b01f7

HOST_FIXUP := autoreconf

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/gengetopt $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/gengetopt
endef

$(eval $(call HostBuild))
