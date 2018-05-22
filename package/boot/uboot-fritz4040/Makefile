#
# Copyright (C) 2013-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_SOURCE_URL:=https://github.com/chunkeey/FritzBox-4040-UBOOT
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=6946ebbaf7b12a4a092d763c8f0c87a25984f103
PKG_SOURCE_DATE:=2017-01-29
PKG_MIRROR_HASH:=5c2394f51a083dca2a2bf9cb36fa717f252112fc792c9eeae64f6383ad08987b

PKG_RELEASE:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=ipq40xx
  UBOOT_IMAGE:=uboot-fritz4040.bin
endef

define U-Boot/fritz4040
  NAME:=FritzBox 4040
endef

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes
UBOOT_MAKE_FLAGS = USE_PRIVATE_LIBGCC=yes
export DTC

define Build/Configure
	$(Build/Configure/U-Boot)
	$(HOSTCC) -o $(PKG_BUILD_DIR)/fritz/lzma2eva $(PKG_BUILD_DIR)/fritz/src/lzma2eva.c -lz
	$(HOSTCC) -o $(PKG_BUILD_DIR)/fritz/tichksum $(PKG_BUILD_DIR)/fritz/src/tichksum.c
	ln -sf $(STAGING_DIR_HOST)/bin/lzma $(PKG_BUILD_DIR)/fritz
	ln -sf compiler-gcc5.h $(PKG_BUILD_DIR)/include/linux/compiler-gcc7.h
endef

define Build/Compile
	$(Build/Compile/U-Boot)
	(cd $(PKG_BUILD_DIR); ./fritz/fritzcreator.sh;)
endef

define Package/u-boot/install
	$(Package/u-boot/install/default)
	$(INSTALL_BIN) ./files/upload-to-f4040.sh $(1)/
endef

UBOOT_TARGETS := fritz4040

$(eval $(call BuildPackage/U-Boot))
