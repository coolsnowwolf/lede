#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=autoconf
PKG_VERSION:=2.71

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/autoconf
PKG_HASH:=431075ad0bf529ef13cb41e9042c542381103e80015686222b8a9d4abef42a1c

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS += \
	EMACS="no" \
	PERL="/usr/bin/env perl"

define Host/Compile
	-$(call Host/Compile/Default,--touch install-man1) # disable building of man pages
	$(call Host/Compile/Default)
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
