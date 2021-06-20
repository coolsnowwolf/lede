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
PKG_SOURCE_DATE:=2021-05-08
PKG_SOURCE_VERSION:=d2c75b2139be003887af9cc5a94da5e9bdc59de7
PKG_MIRROR_HASH:=4af9ce8e11511afee7f588cc982946c06339edbfa47afef6a7f3e2231ac9f34d

PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk

define Trusted-Firmware-A/Default
  BUILD_TARGET:=mediatek
  BUILD_SUBTARGET:=mt7622
  PLAT:=mt7622
  TFA_IMAGE:=bl2.img bl31.bin
  BOOT_DEVICE:=
  DDR3_FLYBY:=
endef

define Trusted-Firmware-A/mt7622-nor-1ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 1x DDR3)
  BOOT_DEVICE:=nor
endef

define Trusted-Firmware-A/mt7622-nor-2ddr
  NAME:=MediaTek MT7622 (SPI-NOR, 2x DDR3)
  BOOT_DEVICE:=nor
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-snand-1ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 1x DDR3)
  BOOT_DEVICE:=snand
endef

define Trusted-Firmware-A/mt7622-snand-2ddr
  NAME:=MediaTek MT7622 (SPI-NAND, 2x DDR3)
  BOOT_DEVICE:=snand
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-emmc-1ddr
  NAME:=MediaTek MT7622 (eMMC, 1x DDR3)
  BOOT_DEVICE:=emmc
endef

define Trusted-Firmware-A/mt7622-emmc-2ddr
  NAME:=MediaTek MT7622 (eMMC, 2x DDR3)
  BOOT_DEVICE:=emmc
  DDR3_FLYBY:=1
endef

define Trusted-Firmware-A/mt7622-sdmmc-1ddr
  NAME:=MediaTek MT7622 (SDcard, 1x DDR3)
  BOOT_DEVICE:=sdmmc
endef

define Trusted-Firmware-A/mt7622-sdmmc-2ddr
  NAME:=MediaTek MT7622 (SDcard, 2x DDR3)
  BOOT_DEVICE:=sdmmc
  DDR3_FLYBY:=1
endef

TFA_TARGETS:= \
	mt7622-nor-1ddr \
	mt7622-nor-2ddr \
	mt7622-snand-1ddr \
	mt7622-snand-2ddr \
	mt7622-emmc-1ddr \
	mt7622-emmc-2ddr \
	mt7622-sdmmc-1ddr \
	mt7622-sdmmc-2ddr

TFA_MAKE_FLAGS += \
	BOOT_DEVICE=$(BOOT_DEVICE) \
	USE_MKIMAGE=1 MKIMAGE=$(STAGING_DIR_HOST)/bin/mkimage \
	$(if $(DDR3_FLYBY),DDR3_FLYBY=1) \
	all

define Build/Configure
	$(call Build/Configure/Default)
endef

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/mt7622/release/bl2.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl2.img
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/build/mt7622/release/bl31.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-bl31.bin
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
