#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bison
PKG_VERSION:=3.5.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=3e7e097bd9709a2d5e40e69446b74b149733b3de864fadb7a9b54eca7b2a4dd0

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --enable-threads=posix --disable-nls

define Host/Clean
	-$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	$(call Host/Clean/Default)
endef

define Host/Install
	$(call Host/Install/Default)
	$(INSTALL_BIN) ./scripts/yacc $(STAGING_DIR_HOST)/bin/yacc
endef

$(eval $(call HostBuild))
