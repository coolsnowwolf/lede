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

PKG_HASH:=0d71e62beb952b41ebafb20a7ee4df2f960db64c31b054721ceb79ff14014c55

FIRMWARE_LANTIQ_SOURCE:=$(TOPDIR)/target/linux/lantiq/files/firmware/lantiq

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk

define U-Boot/Default
  BUILD_TARGET:=lantiq
  DDR_SETTINGS:=
endef

define U-Boot/arv4519pw_ram
  NAME:=Arcadyan arv4519pw (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4519pw
  DDR_SETTINGS:=board/arcadyan/arv4519pw/ddr_settings.h
endef

define U-Boot/arv4519pw_nor
  NAME:=Arcadyan arv4519pw (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4519pw
endef

define U-Boot/arv4519pw_brn
  NAME:=Arcadyan arv4519pw (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4519pw
endef

define U-Boot/arv7506pw11_ram
  NAME:=Arcadyan ARV7506PW11 (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7506pw11
  DDR_SETTINGS:=board/arcadyan/arv7506pw11/ddr_settings.h
endef

define U-Boot/arv7506pw11_nor
  NAME:=Arcadyan ARV7506PW11 (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7506pw11
endef

define U-Boot/arv7506pw11_brn
  NAME:=Arcadyan ARV7506PW11 (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7506pw11
endef

define U-Boot/arv7510pw_ram
  NAME:=Arcadyan arv7510pw (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4510pw
  DDR_SETTINGS:=board/arcadyan/arv7510pw/ddr_settings.h
endef

define U-Boot/arv7510pw_nor
  NAME:=Arcadyan arv7510pw (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4510pw
endef

define U-Boot/arv7510pw_brn
  NAME:=Arcadyan arv7510pw (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv4510pw
endef

define U-Boot/arv7510pw22_ram
  NAME:=Arcadyan arv7510pw22 (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7510pw22
  DDR_SETTINGS:=board/arcadyan/arv7510pw22/ddr_settings.h
endef

define U-Boot/arv7510pw22_nor
  NAME:=Arcadyan arv7510pw22 (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7510pw22
endef

define U-Boot/arv7510pw22_brn
  NAME:=Arcadyan arv7510pw22 (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7510pw22
endef

define U-Boot/arv7518pw_ram
  NAME:=Arcadyan arv7518pw (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7518pw
  DDR_SETTINGS:=board/arcadyan/arv7518pw/ddr_settings.h
endef

define U-Boot/arv7518pw_nor
  NAME:=Arcadyan arv7518pw (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7518pw
endef

define U-Boot/arv7518pw_brn
  NAME:=Arcadyan arv7518pw (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv7518pw
endef

define U-Boot/arv752dpw_ram
  NAME:=Arcadyan arv752dpw (RAM)
  BUILD_SUBTARGET:=xway
  DDR_SETTINGS:=board/arcadyan/arv752dpw/ddr_settings.h
  BUILD_DEVICES:=arcadyan_arv752dpw
endef

define U-Boot/arv752dpw_nor
  NAME:=Arcadyan arv752dpw (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv752dpw
endef

define U-Boot/arv752dpw_brn
  NAME:=Arcadyan arv752dpw (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv752dpw
endef

define U-Boot/arv752dpw22_ram
  NAME:=Arcadyan arv752dpw22 (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv752dpw22
  DDR_SETTINGS:=board/arcadyan/arv752dpw22/ddr_settings.h
endef

define U-Boot/arv752dpw22_nor
  NAME:=Arcadyan arv752dpw22 (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv752dpw22
endef

define U-Boot/arv752dpw22_brn
  NAME:=Arcadyan arv752dpw22 (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv752dpw22
endef

define U-Boot/arv8539pw22_ram
  NAME:=Speedport W 504V Typ A (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv8539pw22
  DDR_SETTINGS:=board/arcadyan/arv8539pw22/ddr_settings.h
endef

define U-Boot/arv8539pw22_nor
  NAME:=Speedport W 504V Typ A (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv8539pw22
endef

define U-Boot/arv8539pw22_brn
  NAME:=Speedport W 504V Typ A (BRN)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=arcadyan_arv8539pw22
endef

define U-Boot/gigasx76x_ram
  NAME:=Siemens Gigaset sx76x (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=siemens_gigaset-sx76x
  DDR_SETTINGS:=board/gigaset/sx76x/ddr_settings.h
endef

define U-Boot/gigasx76x_nor
  NAME:=Siemens Gigaset sx76x (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=siemens_gigaset-sx76x
endef

define U-Boot/acmp252_ram
  NAME:=AudioCodes MP-252 (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=audiocodes_mp-252
  DDR_SETTINGS:=board/audiocodes/acmp252/ddr_settings.h
endef

define U-Boot/acmp252_nor
  NAME:=AudioCodes MP-252 (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=audiocodes_mp-252
endef

define U-Boot/bthomehubv5a_ram
  NAME:=BT Home Hub 5A (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=bt_homehub-v5a
  DDR_SETTINGS:=board/bt/bthomehubv5a/ddr_settings.h
endef

define U-Boot/easy50712_ram
  NAME:=Lantiq EASY50712 (RAM)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=lantiq_easy50712
  DDR_SETTINGS:=board/lantiq/easy50712/ddr_settings.h
endef

define U-Boot/easy50712_nor
  NAME:=Lantiq EASY50712 (NOR)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=lantiq_easy50712
endef

define U-Boot/easy50712_norspl
  NAME:=Lantiq EASY50712 (NOR SPL)
  BUILD_SUBTARGET:=xway
  BUILD_DEVICES:=lantiq_easy50712
  UBOOT_IMAGE:=u-boot.ltq.lzo.norspl
  DEPENDS+=@BROKEN
endef

define U-Boot/easy80920_ram
  NAME:=Lantiq EASY80920 (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=lantiq_easy80920-nor lantiq_easy80920-nand
  DDR_SETTINGS:=board/lantiq/easy80920/ddr_settings.h
endef

define U-Boot/easy80920_nor
  NAME:=Lantiq EASY80920 (NOR)
  BUILD_DEVICES:=lantiq_easy80920-nor lantiq_easy80920-nand
  BUILD_SUBTARGET:=xrx200
endef

define U-Boot/easy80920_norspl
  NAME:=Lantiq EASY80920 (NOR SPL)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=lantiq_easy80920-nor lantiq_easy80920-nand
  UBOOT_IMAGE:=u-boot.ltq.lzo.norspl
  DEPENDS+=@BROKEN
endef

define U-Boot/easy80920_sfspl
  NAME:=Lantiq EASY80920 (SPI SPL)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=lantiq_easy80920-nor lantiq_easy80920-nand
  UBOOT_IMAGE:=u-boot.ltq.lzo.sfspl
  DEPENDS+=@BROKEN
endef

define U-Boot/fb3370_eva
  NAME:=AVM FRITZ3370 (EVA)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=avm_fritz3370
endef

define U-Boot/fb3370_ram
  NAME:=AVM FRITZ3370 (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=avm_fritz3370
  DDR_SETTINGS:=board/avm/fb3370/ddr_settings.h
endef

define U-Boot/fb3370_sfspl
  NAME:=AVM FRITZ3370 (SPI SPL)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=avm_fritz3370
  UBOOT_IMAGE:=u-boot.ltq.lzo.sfspl
  DEPENDS+=@BROKEN
endef

define U-Boot/p2812hnufx_ram
  NAME:=ZyXEL P-2812HNU-Fx (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=zyxel_p-2812hnu-f1
  DDR_SETTINGS:=board/zyxel/p2812hnufx/ddr_settings.h
endef

define U-Boot/p2812hnufx_nandspl
  NAME:=ZyXEL P-2812HNU-Fx (NAND SPL)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=zyxel_p-2812hnu-f1
  UBOOT_IMAGE:=u-boot.ltq.lzo.nandspl
  DEPENDS+=@BROKEN
endef

define U-Boot/vgv7510kw22_brn
  NAME:=Arcadyan VGV7510KW22 (BRN)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7510kw22-nor
endef

define U-Boot/vgv7510kw22_nor
  NAME:=Arcadyan VGV7510KW22 (NOR)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7510kw22-nor
endef

define U-Boot/vgv7510kw22_ram
  NAME:=Arcadyan VGV7510KW22 (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7510kw22-nor
  DDR_SETTINGS:=board/arcadyan/vgv7510kw22/ddr_settings.h
endef

define U-Boot/vgv7519_brn
  NAME:=Arcadyan VGV7519 (BRN)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7519-nor arcadyan_vgv7519-brn
endef

define U-Boot/vgv7519_nor
  NAME:=Arcadyan VGV7519 (NOR)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7519-nor arcadyan_vgv7519-brn
endef

define U-Boot/vgv7519_ram
  NAME:=Arcadyan VGV7519 (RAM)
  BUILD_SUBTARGET:=xrx200
  BUILD_DEVICES:=arcadyan_vgv7519-nor arcadyan_vgv7519-brn
  DDR_SETTINGS:=board/arcadyan/vgv7519/ddr_settings.h
endef

UBOOT_TARGETS:= \
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

UBOOT_MAKE_FLAGS :=

ifeq ($(SUBTARGET),xway)
  SOC:=danube
else
  SOC:=vr9
endif

define Package/u-boot/install/uart
	awk -f $(PKG_BUILD_DIR)/tools/lantiq_ram_init_uart.awk \
		-v soc=$(SOC) $(PKG_BUILD_DIR)/$(DDR_SETTINGS) \
		> $(PKG_BUILD_DIR)/ddr_settings
	perl $(PKG_BUILD_DIR)/tools/gct.pl \
		$(PKG_BUILD_DIR)/ddr_settings $(PKG_BUILD_DIR)/u-boot.srec \
		$(1)/u-boot.asc
endef

define Package/u-boot/install
	$(Package/u-boot/install/$(if $(DDR_SETTINGS),uart,default))
endef

$(eval $(call BuildPackage/U-Boot))
