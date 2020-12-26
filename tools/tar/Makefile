#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=tar
PKG_CPE_ID:=cpe:/a:gnu:tar
PKG_VERSION:=1.32

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/tar
PKG_HASH:=d0d3ae07f103323be809bc3eac0dcc386d52c5262499fe05511ac4788af1fdd8

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--without-posix-acls \
	--without-selinux \
	--without-xattrs \
	--disable-acl \
	--disable-nls

$(eval $(call HostBuild))
