#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=xz
PKG_VERSION:=5.8.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/tukaani-project/xz/releases/download/v$(PKG_VERSION) \
                @SF/lzmautils \
                http://tukaani.org/xz
PKG_HASH:=33bf69c0d6c698e83a68f77e6c1f465778e418ca0b3d59860d3ab446f4ac99a6
PKG_CPE_ID:=cpe:/a:tukaani:xz

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOSTCC := $(HOSTCC_NOCACHE)
HOSTCXX := $(HOSTCXX_NOCACHE)

HOST_CONFIGURE_ARGS += \
	--enable-static=yes \
	--enable-shared=no \
	--disable-doc \
	--disable-nls \
	--with-pic

define Host/Install
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) install xzlinks="unxz xzcat"
endef

define Host/Uninstall
	-$(call Host/Compile/Default,uninstall)
endef

$(eval $(call HostBuild))
