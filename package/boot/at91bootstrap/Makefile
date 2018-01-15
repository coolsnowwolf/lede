#
# Copyright (C) 2016 Microchip Technology Inc.
#     <Sandeepsheriker.mallikarjun@microchip.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=at91bootstrap
PKG_VERSION:=v3.8.8
PKG_RELEASE:=

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/linux4sam/at91bootstrap.git
PKG_SOURCE_VERSION:=3d33a4e0707c61007a5278f6620453502f7500db

PKG_BUILD_DIR = \
    $(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

include at91bootstrap.mk
include $(INCLUDE_DIR)/package.mk

define AT91Bootstrap/Default
  BUILD_TARGET:=at91
  BUILD_SUBTARGET:=sama5
  HIDDEN:=1
  AT91BOOTSTRAP_IMAGE:=at91bootstrap.bin
endef

define AT91Bootstrap/sama5d2_xplaineddf_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (SPI Flash)
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d2_xplaineddf_qspi_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (QSPI Flash)
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d2_xplainedsd_uboot
  TITLE:=AT91Bootstrap for SAMA5D2 Xplained board (SDcard/EMMC)
  BUILD_DEVICES:=at91-sama5d2_xplained
endef

define AT91Bootstrap/sama5d3_xplainednf_uboot
  TITLE:=AT91Bootstrap for SAMA5D3 Xplained board (Nand Flash)
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define AT91Bootstrap/sama5d3_xplainedsd_uboot
  TITLE:=AT91Bootstrap for SAMA5D3 Xplained board (SDcard)
  BUILD_DEVICES:=at91-sama5d3_xplained
endef

define AT91Bootstrap/sama5d4_xplainednf_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (Nand Flash)
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define AT91Bootstrap/sama5d4_xplaineddf_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (SPI Flash)
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

define AT91Bootstrap/sama5d4_xplainedsd_uboot_secure
  TITLE:=AT91Bootstrap for SAMA5D4 Xplained board (SDcard)
  BUILD_DEVICES:=at91-sama5d4_xplained
endef

AT91BOOTSTRAP_TARGETS := \
    sama5d2_xplaineddf_uboot \
    sama5d2_xplaineddf_qspi_uboot \
    sama5d2_xplainedsd_uboot \
    sama5d3_xplainednf_uboot \
    sama5d3_xplainedsd_uboot \
    sama5d4_xplainednf_uboot_secure \
    sama5d4_xplaineddf_uboot_secure \
    sama5d4_xplainedsd_uboot_secure

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
        CROSS_COMPILE=$(TARGET_CROSS)
endef

$(eval $(call BuildPackage/AT91Bootstrap))
