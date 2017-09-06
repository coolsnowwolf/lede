# 
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=xz
PKG_VERSION:=5.2.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/lzmautils \
		http://tukaani.org/xz
PKG_HASH:=fd9ca16de1052aac899ad3495ad20dfa906c27b4a5070102a2ec35ca3a4740c1

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--enable-static=yes \
	--enable-shared=no \

define Host/Prepare
	$(call Host/Prepare/Default)

	# FreeBSD portability fix
	sed -i -e 's|#if defined(HAVE_BYTESWAP_H)|& \&\& !defined(__FreeBSD__)|' \
		$(HOST_BUILD_DIR)/src/common/tuklib_integer.h
endef

define Host/Install
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) install xzlinks="unxz xzcat"
endef

$(eval $(call HostBuild))
