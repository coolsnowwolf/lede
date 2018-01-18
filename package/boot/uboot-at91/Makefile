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
  HIDDEN:=1
  UBOOT_IMAGE:=u-boot.bin boot.bin
endef

define U-Boot/at91sam9m10g45ek_nandflash
  NAME:=AT91SAM9M10G45-EK board (NandFlash)
  BUILD_SUBTARGET:=legacy
  BUILD_DEVICES:=at91sam9m10g45ek
endef

define U-Boot/at91sam9x5ek_nandflash
  NAME:=AT91SAM9X5-EK board (NandFlash)
  BUILD_SUBTARGET:=legacy
  BUILD_DEVICES:=at91sam9g15ek at91sam9g25ek at91sam9g35ek at91sam9x25ek at91sam9x35ek
endef

define U-Boot/sama5d3_xplained_nandflash
  NAME:=SAMA5D3 Xplained board (NandFlash)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define U-Boot/sama5d3_xplained_mmc
  NAME:=SAMA5D3 Xplained board (SDcard)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define U-Boot/sama5d2_xplained_spiflash
  NAME:=SAMA5D2 Xplained board (SPI Flash)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define U-Boot/sama5d2_xplained_mmc
  NAME:=SAMA5D2 Xplained board (SDcard/EMMC)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define U-Boot/sama5d4_xplained_spiflash
  NAME:=SAMA5D4 Xplained board (SPI Flash)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define U-Boot/sama5d4_xplained_mmc
  NAME:=SAMA5D4 Xplained board (SDcard)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define U-Boot/sama5d4_xplained_nandflash
  NAME:=SAMA5D4 Xplained board (NandFlash)
  BUILD_SUBTARGET:=sama5
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

UBOOT_TARGETS := \
	at91sam9m10g45ek_nandflash \
	at91sam9x5ek_nandflash \
	sama5d3_xplained_nandflash \
	sama5d3_xplained_mmc \
	sama5d2_xplained_mmc \
	sama5d2_xplained_spiflash \
	sama5d4_xplained_mmc \
	sama5d4_xplained_spiflash \
	sama5d4_xplained_nandflash

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		KCFLAGS="$(filter-out -fstack-protector -mfloat-abi=hard, $(TARGET_CFLAGS)) -mfloat-abi=soft" 
endef

$(eval $(call BuildPackage/U-Boot))
