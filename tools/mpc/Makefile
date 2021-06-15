#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpc
PKG_VERSION:=1.2.0

PKG_SOURCE_URL:=@GNU/mpc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=e90f2d99553a9c19911abdb4305bf8217106a957e3994436428572c8dfe8fda6

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport CFLAGS

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-mpfr=$(TOPDIR)/staging_dir/host \
	--with-gmp=$(TOPDIR)/staging_dir/host

$(eval $(call HostBuild))
