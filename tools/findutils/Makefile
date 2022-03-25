# 
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=findutils
PKG_CPE_ID:=cpe:/a:gnu:findutils
PKG_VERSION:=4.9.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_HASH:=a2bfb8c09d436770edc59f50fa483e785b161a3b7b9d547573cb08065fd462fe

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

$(eval $(call HostBuild))
