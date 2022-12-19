#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpc
PKG_VERSION:=1.3.1

PKG_SOURCE_URL:=@GNU/mpc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=ab642492f5cf882b74aa0cb730cd410a81edcdbec895183ce930e706c1c759b8

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport CFLAGS

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--with-mpfr=$(TOPDIR)/staging_dir/host \
	--with-gmp=$(TOPDIR)/staging_dir/host

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
