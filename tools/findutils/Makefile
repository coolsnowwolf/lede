# 
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=findutils
PKG_CPE_ID:=cpe:/a:gnu:findutils
PKG_VERSION:=4.10.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=1387e0b67ff247d2abde998f90dfbf70c1491391a59ddfecb8ae698789f0a4f5

PKG_PROGRAMS:=find locate updatedb xargs

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

ifeq ($(HOST_OS),Darwin)
	HOST_CFLAGS +=-D__nonnull\\(params\\)=
endif

HOST_CONFIGURE_ARGS += \
	--enable-threads=pth \
	--disable-rpath \
	--disable-nls \
	--without-selinux

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
