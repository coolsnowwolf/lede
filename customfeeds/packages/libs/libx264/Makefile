#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=x264
PKG_VERSION:=2020-10-26
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://code.videolan.org/videolan/x264.git
PKG_SOURCE_VERSION:=4121277b40a667665d4eea1726aefdc55d12d110
PKG_MIRROR_HASH:=4b4955e8f92d0c4afecbced2cc6414a123085f7472d198b3eeddaa9490b84f60

PKG_MAINTAINER:=Adrian Panella <ianchi74@outlook.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS:=$(filter-out -O%,$(TARGET_CFLAGS))
MAKE_FLAGS:=$(filter-out LD=%,$(MAKE_FLAGS)) LD="$(TARGET_CC) -o"

# Instead of blacklisting a boatload of platforms just enable
# optimization where it makes sense (matters) ie ARMv7+ and x86_64

ifneq ($(findstring cortex-a,$(CPU_TYPE)),)
  CONFIGURE_ARGS += --enable-lto
  else
  ifneq ($(CONFIG_TARGET_x86_64),)
    CONFIGURE_ARGS += --enable-lto
    ifeq ($(CONFIG_NASM),y)
      CONFIGURE_VARS+= AS=nasm
      MAKE_FLAGS+= AS=nasm
    endif
    else
    CONFIGURE_VARS+= AS=
    MAKE_FLAGS+= AS=
    CONFIGURE_ARGS += --disable-asm
  endif
endif

CONFIGURE_ARGS += \
		--disable-cli \
		--enable-shared \
		--disable-opencl \
		--enable-pic \
		--disable-avs \
		--disable-ffms \
		--disable-gpac \
		--disable-lsmash

define Package/libx264
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=H264/AVC free codec library
  DEPENDS:=+libpthread @BUILD_PATENTED
  URL:=https://www.videolan.org/developers/x264.html
endef

define Package/libx264/description
  x264 is a free software library for encoding
  video streams into the H.264/MPEG-4 AVC compression format.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/x264/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig/
endef

define Package/libx264/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libx264))
