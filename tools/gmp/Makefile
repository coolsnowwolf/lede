#
# Copyright (C) 2009-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gmp
PKG_VERSION:=6.3.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/gmp/
PKG_HASH:=e56fd59d76810932a0555aa15a14b61c16bed66110d3c75cc2ac49ddaa9ab24c
PKG_CPE_ID:=cpe:/a:gmplib:gmp

HOST_FIXUP:=autoreconf

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

unexport CFLAGS

HOST_CONFIGURE_ARGS += \
	--enable-static \
	--disable-shared \
	--disable-assembly \
	--enable-cxx

ifeq ($(GNU_HOST_NAME),x86_64-linux-gnux32)
HOST_CONFIGURE_ARGS += ABI=x32
endif

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
