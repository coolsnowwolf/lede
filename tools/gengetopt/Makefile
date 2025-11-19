#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gengetopt
PKG_VERSION:=2.23

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=b941aec9011864978dd7fdeb052b1943535824169d2aa2b0e7eae9ab807584ac

PKG_SUBDIRS:=$$$$(foreach dir,gl src skels,$$$$(wildcard $$$$(dir)) )

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	$(call Host/Compile/Default,SUBDIRS='$(PKG_SUBDIRS)')
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/gengetopt $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/gengetopt
endef

$(eval $(call HostBuild))
