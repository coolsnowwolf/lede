#
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libressl
PKG_VERSION:=3.3.3
PKG_HASH:=a471565b36ccd1a70d0bd7d37c6e95c43a26a62829b487d9d2cdebfe58be3066
PKG_RELEASE:=1

PKG_CPE_ID:=cpe:/a:openbsd:libressl

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://mirror.ox.ac.uk/pub/OpenBSD/LibreSSL \
	http://ftp.jaist.ac.jp/pub/OpenBSD/LibreSSL \
	https://ftp.openbsd.org/pub/OpenBSD/LibreSSL

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOST_CONFIGURE_ARGS += --enable-static --disable-shared --disable-tests
HOST_CFLAGS += $(HOST_FPIC)

ifeq ($(GNU_HOST_NAME),x86_64-linux-gnux32)
HOST_CONFIGURE_ARGS += --disable-asm
endif

$(eval $(call HostBuild))
