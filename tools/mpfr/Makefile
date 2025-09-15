#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpfr
PKG_VERSION:=4.2.1
PKG_CPE_ID:=cpe:/a:mpfr:gnu_mpfr

PKG_SOURCE_URL:=@GNU/mpfr http://www.mpfr.org/mpfr-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=116715552bd966c85b417c424db1bbdf639f53836eb361549d1f8d6ded5cb4c6

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--enable-thread-safe \
	--with-gmp=$(STAGING_DIR_HOST)

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
