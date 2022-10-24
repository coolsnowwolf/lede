#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mpc
PKG_VERSION:=1.2.1

PKG_SOURCE_URL:=@GNU/mpc/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_HASH:=17503d2c395dfcf106b622dc142683c1199431d095367c6aacba6eec30340459

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
