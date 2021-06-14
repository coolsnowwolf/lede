#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_CPE_ID:=cpe:/a:gnu:grub2
PKG_VERSION:=2.06~rc1
PKG_RELEASE:=2

PKG_SOURCE:=grub-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://alpha.gnu.org/gnu/grub
PKG_HASH:=2c87f1f21e2ab50043e6cd9163c08f1b6c3a6171556bf23ff9ed65b074145484

HOST_BUILD_PARALLEL:=1

PKG_ASLR_PIE:=0
PKG_SSP:=0

PKG_FLAGS:=nonshared

PATCH_DIR := ../patches
HOST_PATCH_DIR := ../patches
HOST_BUILD_DIR := $(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/grub2/Default
  CATEGORY:=Boot Loaders
  SECTION:=boot
  TITLE:=GRand Unified Bootloader
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86||TARGET_x86_64
endef

HOST_BUILD_PREFIX := $(STAGING_DIR_HOST)

CONFIGURE_VARS += \
	grub_build_mkfont_excuse="don't want fonts"

CONFIGURE_ARGS += \
	--target=$(REAL_GNU_TARGET_NAME) \
	--disable-werror \
	--disable-nls \
	--disable-device-mapper \
	--disable-libzfs \
	--disable-liblzma \
	--disable-grub-mkfont \
	--with-platform=none

HOST_CONFIGURE_VARS += \
	grub_build_mkfont_excuse="don't want fonts"

HOST_CONFIGURE_ARGS += \
	--disable-grub-mkfont \
	--target=$(REAL_GNU_TARGET_NAME) \
	--sbindir="$(STAGING_DIR_HOST)/bin" \
	--disable-werror \
	--disable-libzfs \
	--disable-nls

HOST_MAKE_FLAGS += \
	TARGET_RANLIB=$(TARGET_RANLIB) \
	LIBLZMA=$(STAGING_DIR_HOST)/lib/liblzma.a

TARGET_CFLAGS := $(filter-out -O2 -O3 -fno-plt,$(TARGET_CFLAGS))

define Host/Configure
	$(SED) 's,(RANLIB),(TARGET_RANLIB),' $(HOST_BUILD_DIR)/grub-core/Makefile.in
	$(Host/Configure/Default)
endef
