#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_VERSION:=1.5.2

PKG_SOURCE_VERSION:=aea36417067dade75192bafa03af70b6eb2677b1
PKG_SOURCE_URL:=git://git.infradead.org/mtd-utils.git
PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=e11b342b85a36b2e438a8412ec52f87621d3046aec1a93039f8c72de9990b2a7

include $(INCLUDE_DIR)/host-build.mk

HOST_CFLAGS += -I$(STAGING_DIR_HOST)/include/e2fsprogs
CFLAGS := $(HOST_CFLAGS) -I$(HOST_BUILD_DIR)/include -L$(HOST_BUILD_DIR) -L$(STAGING_DIR_HOST)/lib -DNO_NATIVE_SUPPORT
ifneq ($(HOST_OS),Linux)
CFLAGS += -Dloff_t=off_t -D__BYTE_ORDER=BYTE_ORDER -include endian.h -I$(CURDIR)/include -include fls.h
MTD_STATIC := 0
else
MTD_STATIC := 1
endif

MTD_MAKEOPTS = \
	CFLAGS="$(CFLAGS)" \
	LDFLAGS="$(HOST_LDFLAGS)" \
	WITHOUT_LZO=1 WITHOUT_XATTR=1 \
	SUBDIRS="" \
	BUILDDIR="$(HOST_BUILD_DIR)" \
	STATIC=$(MTD_STATIC) \
	V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1)

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) \
		$(MTD_MAKEOPTS) \
		TARGETS="mkfs.jffs2 ubi-utils/ubinize mkfs.ubifs/mkfs.ubifs"
endef

define Host/Install
	$(CP) \
		$(HOST_BUILD_DIR)/mkfs.jffs2 \
		$(HOST_BUILD_DIR)/mkfs.ubifs/mkfs.ubifs \
		$(HOST_BUILD_DIR)/ubi-utils/ubinize \
		$(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/{mkfs.jffs2,mkfs.ubifs,ubinize}
endef

$(eval $(call HostBuild))
