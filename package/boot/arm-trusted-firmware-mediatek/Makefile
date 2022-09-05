#
# Copyright (C) 2017 Hauke Mehrtens
# Copyright (C) 2021 Daniel Golle
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-mediatek
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/mtk-openwrt/arm-trusted-firmware.git
PKG_SOURCE_DATE:=2022-08-31
PKG_SOURCE_VERSION:=7539348480af57c6d0db95aba6381f3ee7483779
PKG_MIRROR_HASH:=125090124d77753acc379b3b124100978c1ecb3da37c4983ba9644b433b7eb08

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=mediatek
  TFA_IMAGE:=bl2.img bl31.bin
  BOOT_DEVICE:=
  DDR3_FLYBY:=
  DDR_TYPE:=
  NAND_TYPE:=
endef

define Trusted-Firmware-A/mt7622-nor-1ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=nor
endef

define Trusted-Firmware-A/mt7622-nor-2ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=nor
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-snand-1ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
endef

define Trusted-Firmware-A/mt7622-snand-2ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=snand
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-emmc-1ddr
  NAME:=MediaTek MT7622 (eMMC, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=emmc
endef

define Trusted-Firmware-A/mt7622-emmc-2ddr
  NAME:=MediaTek MT7622 (eMMC, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=emmc
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-sdmmc-1ddr
  NAME:=MediaTek MT7622 (SD card, 1x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=sdmmc
endef

define Trusted-Firmware-A/mt7622-sdmmc-2ddr
  NAME:=MediaTek MT7622 (SD card, 2x DDR3)
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  BOOT_DEVICE:=sdmmc
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7986-nor-ddr4
  NAME:=MediaTek MT7986 (SPI-NOR, DDR4)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-emmc-ddr4
  NAME:=MediaTek MT7986 (eMMC, DDR4)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-sdmmc-ddr4
  NAME:=MediaTek MT7986 (SD card, DDR4)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-snand-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SNFI, DDR4)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
endef

define Trusted-Firmware-A/mt7986-spim-nand-ddr4
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM, DDR4)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr4
  NAND_TYPE:=spim:2k+64
endef

define Trusted-Firmware-A/mt7986-nor-ddr3
  NAME:=MediaTek MT7986 (SPI-NOR, DDR3)
  BOOT_DEVICE:=nor
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-emmc-ddr3
  NAME:=MediaTek MT7986 (eMMC, DDR3)
  BOOT_DEVICE:=emmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-sdmmc-ddr3
  NAME:=MediaTek MT7986 (SD card, DDR3)
  BOOT_DEVICE:=sdmmc
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-snand-ddr3
  NAME:=MediaTek MT7986 (SPI-NAND via SNFI, DDR3)
  BOOT_DEVICE:=snand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

define Trusted-Firmware-A/mt7986-spim-nand-ddr3
  NAME:=MediaTek MT7986 (SPI-NAND via SPIM, DDR3)
  BOOT_DEVICE:=spim-nand
  BUILD_SUBTARGET:=filogic
  PLAT:=mt7986
  DDR_TYPE:=ddr3
endef

TFA_TARGETS:= \
	mt7622-nor-1ddr \
	mt7622-nor-2ddr \
	mt7622-snand-1ddr \
	mt7622-snand-2ddr \
	mt7622-emmc-1ddr \
	mt7622-emmc-2ddr \
	mt7622-sdmmc-1ddr \
	mt7622-sdmmc-2ddr \
	mt7986-emmc-ddr3 \
	mt7986-nor-ddr3 \
	mt7986-sdmmc-ddr3 \
	mt7986-snand-ddr3 \
	mt7986-spim-nand-ddr3 \
	mt7986-emmc-ddr4 \
	mt7986-nor-ddr4 \
	mt7986-sdmmc-ddr4 \
	mt7986-snand-ddr4 \
	mt7986-spim-nand-ddr4

TFA_MAKE_FLAGS += \
	BOOT_DEVICE=$(BOOT_DEVICE) \
	USE_MKIMAGE=1 MKIMAGE=$(STAGING_DIR_HOST)/bin/mkimage \
	$(if $(findstring ddr4,$(DDR_TYPE)),DRAM_USE_DDR4=1) \
	$(if $(NAND_TYPE),NAND_TYPE=$(NAND_TYPE)) \
	HAVE_DRAM_OBJ_FILE=yes \
	$(if $(DDR3_FLYBY),DDR3_FLYBY=1) \
	all

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl2.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.img
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/$(PLAT)/release/bl31.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl31.bin
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
