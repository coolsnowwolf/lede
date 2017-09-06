#
# Copyright (C) 2016 Ben Whitten <ben.whitten@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_VERSION:=2016.05
PKG_RELEASE:=1

PKG_HASH:=87d02275615aaf0cd007b54cbe9fbadceef2bee7c79e6c323ea1ae8956dcb171

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=at91
  BUILD_SUBTARGET:=legacy
  UBOOT_IMAGE:=u-boot.bin boot.bin
endef

define U-Boot/at91sam9m10g45ek_nandflash
  NAME:=AT91SAM9M10G45-EK board (NandFlash)
endef

define U-Boot/at91sam9x5ek_nandflash
  NAME:=AT91SAM9X5-EK board (NandFlash)
endef

UBOOT_TARGETS := \
	at91sam9m10g45ek_nandflash \
	at91sam9x5ek_nandflash

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		KCFLAGS="$(filter-out -fstack-protector, $(TARGET_CFLAGS))"
endef

$(eval $(call BuildPackage/U-Boot))
