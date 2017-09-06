#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpfr
PKG_VERSION:=3.1.5

PKG_SOURCE_URL:=http://www.mpfr.org/mpfr-$(PKG_VERSION) \
		@GNU/mpfr
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_HASH:=015fde82b3979fbe5f83501986d328331ba8ddf008c1ff3da3c238f49ca062bc

HOST_BUILD_PARALLEL:=1
HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-gmp=$(STAGING_DIR_HOST)

$(eval $(call HostBuild))
