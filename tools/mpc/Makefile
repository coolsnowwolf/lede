#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpc
PKG_VERSION:=1.0.3

PKG_SOURCE_URL:=@GNU/mpc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=617decc6ea09889fb08ede330917a00b16809b8db88c29c31bfbb49cbf88ecc3

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport CFLAGS

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-mpfr=$(TOPDIR)/staging_dir/host \
	--with-gmp=$(TOPDIR)/staging_dir/host

$(eval $(call HostBuild))
