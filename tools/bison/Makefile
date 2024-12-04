#
# Copyright (C) 2008-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=bison
PKG_VERSION:=3.8.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=06c9e13bdf7eb24d4ceb6b59205a4f67c2c7e7213119644430fe82fbd14a0abb
PKG_CPE_ID:=cpe:/a:gnu:bison

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --enable-threads=posix --disable-nls

define Host/Install
	$(call Host/Install/Default)
	$(INSTALL_BIN) ./scripts/yacc $(STAGING_DIR_HOST)/bin/yacc
endef

define Host/Uninstall
	rm -f $(STAGING_DIR_HOST)/bin/yacc
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
