#
# Copyright (C) 2016 Microchip Technology Inc.
#     <Sandeepsheriker.mallikarjun@microchip.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=at91bootstrap
PKG_VERSION:=v3.8.10
PKG_RELEASE:=

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/linux4sam/at91bootstrap.git
PKG_SOURCE_VERSION:=04efa5500d60a0211d14b6ee60df7ce0a828704d

PKG_BUILD_DIR = \
    $(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include at91bootstrap.mk
include $(INCLUDE_DIR)/package.mk

define AT91Bootstrap/Default
  BUILD_TARGET:=at91
  HIDDEN:=1
  AT91BOOTSTRAP_IMAGE:=at91bootstrap.bin
endef

define AT91Bootstrap/sama5d2_xplaineddf_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (SPI Flash)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d2_xplaineddf_qspi_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (QSPI Flash)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d2_xplainedsd_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (SDcard/EMMC)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d3_xplainednf_uboot
  TITLE:=AT91Bootstrap for SAMA5D3 Xplained board (Nand Flash)
  BUILD_SUBTARGET:=sama5d3
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define AT91Bootstrap/sama5d3_xplainedsd_uboot
  TITLE:=AT91Bootstrap for SAMA5D3 Xplained board (SDcard)
  BUILD_SUBTARGET:=sama5d3
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define AT91Bootstrap/sama5d4_xplainednf_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (Nand Flash)
  BUILD_SUBTARGET:=sama5d4
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define AT91Bootstrap/sama5d4_xplaineddf_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (SPI Flash)
  BUILD_SUBTARGET:=sama5d4
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define AT91Bootstrap/sama5d4_xplainedsd_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (SDcard)
  BUILD_SUBTARGET:=sama5d4
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define AT91Bootstrap/sama5d27_som1_eksd_uboot
  TITLE:=AT91Bootstrap for SAMA5D27 SOM1 Ek (SDcard0)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d27_som1_ek
endef

define AT91Bootstrap/sama5d27_som1_eksd1_uboot
  TITLE:=AT91Bootstrap for SAMA5D27 SOM1 Ek (SDcard1)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d27_som1_ek
endef

define AT91Bootstrap/sama5d27_som1_ekqspi_uboot
  TITLE:=AT91Bootstrap for SAMA5D27 SOM1 Ek (QSPI Flash)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d27_som1_ek
endef

define AT91Bootstrap/sama5d2_ptc_eknf_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 PTC EK (Nand Flash)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d2_ptc_ek
endef

define AT91Bootstrap/sama5d2_ptc_eksd_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 PTC EK (SDCard)
  BUILD_SUBTARGET:=sama5d2
  BUILD_DEVICES:=at91-sama5d2_ptc_ek
endef

AT91BOOTSTRAP_TARGETS := \
    sama5d2_xplaineddf_uboot \
    sama5d2_xplaineddf_qspi_uboot \
    sama5d2_xplainedsd_uboot \
    sama5d3_xplainednf_uboot \
    sama5d3_xplainedsd_uboot \
    sama5d4_xplainednf_uboot_secure \
    sama5d4_xplaineddf_uboot_secure \
    sama5d4_xplainedsd_uboot_secure \
	sama5d27_som1_eksd_uboot \
	sama5d27_som1_ekqspi_uboot \
	sama5d2_ptc_eknf_uboot \
    sama5d2_ptc_eksd_uboot

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
        CROSS_COMPILE=$(TARGET_CROSS)
endef

$(eval $(call BuildPackage/AT91Bootstrap))
