# 
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mtools
PKG_VERSION:=4.0.18

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=30d408d039b4cedcd04fbf824c89b0ff85dcbb6f71f13d2d8d65abb3f58cacc3
PKG_CAT:=zcat

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	ac_cv_header_iconv_h=no

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) mcopy mmd
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mcopy $(STAGING_DIR_HOST)/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mmd $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mcopy
	rm -f $(STAGING_DIR_HOST)/bin/mmd
endef

$(eval $(call HostBuild))
