#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpfr
PKG_VERSION:=4.0.1
PKG_CPE_ID:=cpe:/a:mpfr:gnu_mpfr

PKG_SOURCE_URL:=http://www.mpfr.org/mpfr-$(PKG_VERSION) \
		@GNU/mpfr
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=67874a60826303ee2fb6affc6dc0ddd3e749e9bfcb4c8655e3953d0458a6e16e

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-gmp=$(STAGING_DIR_HOST)

$(eval $(call HostBuild))
