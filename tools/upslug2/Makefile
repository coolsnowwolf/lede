#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=upslug2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/fretb/upslug2
PKG_SOURCE_DATE:=2016-05-16
PKG_SOURCE_VERSION:=35f0b18c56368f69eef590e7d9fb688fbe624b10
PKG_MIRROR_HASH:=646856532e5fad240b1697eb94726aafa6712294f608e24892983e6db95fb65f

HOST_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

ifneq ($(HOST_OS),Linux)
	HOST_CONFIGURE_ARGS += --with-libpcap
endif

$(eval $(call HostBuild))
