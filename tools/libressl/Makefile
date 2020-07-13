#
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libressl
PKG_VERSION:=3.0.2
PKG_HASH:=df7b172bf79b957dd27ef36dcaa1fb162562c0e8999e194aa8c1a3df2f15398e
PKG_RELEASE:=1

PKG_CPE_ID:=cpe:/a:openbsd:libressl

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://mirror.ox.ac.uk/pub/OpenBSD/LibreSSL \
	http://ftp.jaist.ac.jp/pub/OpenBSD/LibreSSL \
	https://ftp.openbsd.org/pub/OpenBSD/LibreSSL

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --disable-shared
HOST_CFLAGS += $(FPIC)

ifeq ($(GNU_HOST_NAME),x86_64-linux-gnux32)
HOST_CONFIGURE_ARGS += --disable-asm
endif

$(eval $(call HostBuild))
