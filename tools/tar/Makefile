#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=tar
PKG_VERSION:=1.29

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@GNU/tar
PKG_HASH:=236b11190c0a3a6885bdb8d61424f2b36a5872869aa3f7f695dea4b4843ae2f2

HOST_BUILD_PARALLEL := 1

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
