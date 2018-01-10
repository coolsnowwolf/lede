#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=u-boot
PKG_VERSION:=2013.10
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://mirror2.openwrt.org/sources \
	ftp://ftp.denx.de/pub/u-boot
PKG_HASH:=0d71e62beb952b41ebafb20a7ee4df2f960db64c31b054721ceb79ff14014c55
PKG_TARGETS:=bin

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

PKG_BUILD_PARALLEL:=1

FIRMWARE_LANTIQ_SOURCE:=$(TOPDIR)/target/linux/lantiq/files/firmware/lantiq

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  SOC:=
  DDR_SETTINGS:=
  IMAGE:=
  DEPS:=
endef

define uboot/arv4519pw_ram
  TITLE:=U-Boot for Arcadyan arv4519pw (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv4519pw/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv4519pw_nor
  TITLE:=U-Boot for Arcadyan arv4519pw (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv4519pw_brn
  TITLE:=U-Boot for Arcadyan arv4519pw (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7506pw11_ram
  TITLE:=U-Boot for Arcadyan ARV7506PW11 (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv7506pw11/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7506pw11_nor
  TITLE:=U-Boot for Arcadyan ARV7506PW11 (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7506pw11_brn
  TITLE:=U-Boot for Arcadyan ARV7506PW11 (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw_ram
  TITLE:=U-Boot for Arcadyan arv7510pw (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv7510pw/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw_nor
  TITLE:=U-Boot for Arcadyan arv7510pw (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw_brn
  TITLE:=U-Boot for Arcadyan arv7510pw (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw22_ram
  TITLE:=U-Boot for Arcadyan arv7510pw22 (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv7510pw22/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw22_nor
  TITLE:=U-Boot for Arcadyan arv7510pw22 (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7510pw22_brn
  TITLE:=U-Boot for Arcadyan arv7510pw22 (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7518pw_ram
  TITLE:=U-Boot for Arcadyan arv7518pw (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv7518pw/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7518pw_nor
  TITLE:=U-Boot for Arcadyan arv7518pw (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv7518pw_brn
  TITLE:=U-Boot for Arcadyan arv7518pw (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw_ram
  TITLE:=U-Boot for Arcadyan arv752dpw (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv752dpw/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw_nor
  TITLE:=U-Boot for Arcadyan arv752dpw (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw_brn
  TITLE:=U-Boot for Arcadyan arv752dpw (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw22_ram
  TITLE:=U-Boot for Arcadyan arv752dpw22 (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv752dpw22/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw22_nor
  TITLE:=U-Boot for Arcadyan arv752dpw22 (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv752dpw22_brn
  TITLE:=U-Boot for Arcadyan arv752dpw22 (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv8539pw22_ram
  TITLE:=U-Boot for Speedport W 504V Typ A (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/arcadyan/arv8539pw22/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv8539pw22_nor
  TITLE:=U-Boot for Speedport W 504V Typ A (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/arv8539pw22_brn
  TITLE:=U-Boot for Speedport W 504V Typ A (BRN)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/gigasx76x_ram
  TITLE:=U-Boot for Siemens Gigaset sx76x (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/gigaset/sx76x/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/gigasx76x_nor
  TITLE:=U-Boot for Siemens Gigaset sx76x (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/acmp252_ram
  TITLE:=U-Boot for AudioCodes MP-252 (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/audiocodes/acmp252/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/acmp252_nor
  TITLE:=U-Boot for AudioCodes MP-252 (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/bthomehubv5a_ram
  TITLE:=U-Boot for BT Home Hub 5A (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/bt/bthomehubv5a/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/easy50712_ram
  TITLE:=U-Boot for Lantiq EASY50712 (RAM)
  SOC:=danube
  DDR_SETTINGS:=board/lantiq/easy50712/ddr_settings.h
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/easy50712_nor
  TITLE:=U-Boot for Lantiq EASY50712 (NOR)
  SOC:=danube
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/easy50712_norspl
  TITLE:=U-Boot for Lantiq EASY50712 (NOR SPL)
  SOC:=danube
  IMAGE:=u-boot.ltq.lzo.norspl
  DEPS:=@TARGET_lantiq_xway
endef

define uboot/easy80920_ram
  TITLE:=U-Boot for Lantiq EASY80920 (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/lantiq/easy80920/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/easy80920_nor
  TITLE:=U-Boot for Lantiq EASY80920 (NOR)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/easy80920_norspl
  TITLE:=U-Boot for Lantiq EASY80920 (NOR SPL)
  SOC:=vr9
  IMAGE:=u-boot.ltq.lzo.norspl
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/easy80920_sfspl
  TITLE:=U-Boot for Lantiq EASY80920 (SPI SPL)
  SOC:=vr9
  IMAGE:=u-boot.ltq.lzo.sfspl
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/fb3370_eva
  TITLE:=U-Boot for AVM FRITZ3370 (EVA)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/fb3370_ram
  TITLE:=U-Boot for AVM FRITZ3370 (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/avm/fb3370/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/fb3370_sfspl
  TITLE:=U-Boot for AVM FRITZ3370 (SPI SPL)
  SOC:=vr9
  IMAGE:=u-boot.ltq.lzo.sfspl
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/p2812hnufx_ram
  TITLE:=U-Boot for ZyXEL P-2812HNU-Fx (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/zyxel/p2812hnufx/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/p2812hnufx_nandspl
  TITLE:=U-Boot for ZyXEL P-2812HNU-Fx (NAND SPL)
  SOC:=vr9
  IMAGE:=u-boot.ltq.lzo.nandspl
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7510kw22_brn
  TITLE:=U-Boot for Arcadyan VGV7510KW22 (BRN)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7510kw22_nor
  TITLE:=U-Boot for Arcadyan VGV7510KW22 (NOR)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7510kw22_ram
  TITLE:=U-Boot for Arcadyan VGV7510KW22 (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/arcadyan/vgv7510kw22/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7519_brn
  TITLE:=U-Boot for Arcadyan VGV7519 (BRN)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7519_nor
  TITLE:=U-Boot for Arcadyan VGV7519 (NOR)
  SOC:=vr9
  DEPS:=@TARGET_lantiq_xrx200
endef

define uboot/vgv7519_ram
  TITLE:=U-Boot for Arcadyan VGV7519 (RAM)
  SOC:=vr9
  DDR_SETTINGS:=board/arcadyan/vgv7519/ddr_settings.h
  DEPS:=@TARGET_lantiq_xrx200
endef

UBOOTS:= \
	arv4519pw_ram arv4519pw_nor arv4519pw_brn \
	arv7506pw11_ram arv7506pw11_nor arv7506pw11_brn \
	arv7510pw_ram arv7510pw_nor arv7510pw_brn \
	arv7510pw22_ram arv7510pw22_nor arv7510pw22_brn \
	arv7518pw_ram arv7518pw_nor arv7518pw_brn \
	arv752dpw_ram arv752dpw_nor arv752dpw_brn \
	arv752dpw22_ram arv752dpw22_nor arv752dpw22_brn \
	arv8539pw22_brn arv8539pw22_nor arv8539pw22_ram \
	bthomehubv5a_ram \
	gigasx76x_ram gigasx76x_nor \
	acmp252_ram acmp252_nor \
	easy50712_ram easy50712_nor easy50712_norspl \
	easy80920_ram easy80920_nor easy80920_norspl easy80920_sfspl \
	fb3370_eva fb3370_ram fb3370_sfspl \
	p2812hnufx_ram p2812hnufx_nandspl \
	vgv7510kw22_brn vgv7510kw22_nor vgv7510kw22_ram \
	vgv7519_brn vgv7519_nor vgv7519_ram

define Package/uboot/template
define Package/uboot-lantiq-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=$(3) @!IN_SDK
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
  MAINTAINER:=Luka Perkov <luka@openwrt.org>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	DEPS:=$(uboot/$(1)/DEPS)
	$(call Package/uboot/template,$(1),$(TITLE),$(DEPS))
endef

define CompressVR9Firmware
	$(STAGING_DIR_HOST)/bin/lzma e \
		$(FIRMWARE_LANTIQ_SOURCE)/vr9_phy$(1)_a$(2)x.bin \
		$(PKG_BUILD_DIR)/arch/mips/cpu/mips32/vrx200/fw_phy$(1)_a$(2)x.blob
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)/arch/mips/cpu/mips32/vrx200/
	$(call CompressVR9Firmware,11g,1)
	$(call CompressVR9Firmware,11g,2)
	$(call CompressVR9Firmware,22f,1)
	$(call CompressVR9Firmware,22f,2)
endef

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) $(BUILD_VARIANT)_config
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/uboot/install/default
	$(CP) \
		$(PKG_BUILD_DIR)/$(2) \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.img
endef

define Package/uboot/install/uart
	awk -f $(PKG_BUILD_DIR)/tools/lantiq_ram_init_uart.awk \
		-v soc=$(2) $(PKG_BUILD_DIR)/$(3) \
		> $(PKG_BUILD_DIR)/ddr_settings
	perl $(PKG_BUILD_DIR)/tools/gct.pl \
		$(PKG_BUILD_DIR)/ddr_settings $(PKG_BUILD_DIR)/u-boot.srec \
		$(BIN_DIR)/uboot-$(BOARD)-$(1)/openwrt-$(BOARD)-$(1)-u-boot.asc
 endef

define Package/uboot/install/template
define Package/uboot-lantiq-$(1)/install
	$(call Package/uboot/install/default,$(1),$(if $(IMAGE),$(IMAGE),u-boot.bin))
	$(if $(DDR_SETTINGS), \
		$(call Package/uboot/install/uart,$(1),$(SOC),$(DDR_SETTINGS)) \
	)
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call Package/uboot/install/template,$(u))) \
	$(eval $(call BuildPackage,uboot-lantiq-$(u))) \
)
