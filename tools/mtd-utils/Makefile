#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_VERSION:=2.1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://infraroot.at/pub/mtd/
PKG_HASH:=386e27fd121699b6b729bc2e8e04dda987b31cca6b16e12fb6cc6dcf26449f46

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk

ifneq ($(HOST_OS),Linux)
  HOST_CFLAGS += \
	-I$(CURDIR)/include \
	-Dloff_t=off_t \
	-D__BYTE_ORDER=BYTE_ORDER \
	-include endian.h \
	-DNO_NATIVE_SUPPORT \
	-include fls.h
endif

HOST_CONFIGURE_VARS+= \
	UUID_CFLAGS="-I$(STAGING_DIR_HOST)/include/e2fsprogs/uuid"

HOST_CONFIGURE_ARGS+= \
	--disable-tests \
	--without-crypto \
	--without-xattr \
	--without-zstd \
	--without-lzo

HOST_MAKE_FLAGS += \
	PROGRAMS="mkfs.jffs2 ubinize mkfs.ubifs"

define Host/Install
	$(CP) \
		$(HOST_BUILD_DIR)/mkfs.jffs2 \
		$(HOST_BUILD_DIR)/mkfs.ubifs \
		$(HOST_BUILD_DIR)/ubinize \
		$(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/{mkfs.jffs2,mkfs.ubifs,ubinize}
endef

$(eval $(call HostBuild))
