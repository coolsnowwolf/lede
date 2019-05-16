#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=brcm2708-gpu-fw
PKG_VERSION:=2018-11-29
PKG_RELEASE:=b428bdd819df8d0ad3009b64492a4b3d1f9453e4

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)/rpi-firmware-$(PKG_RELEASE)

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

RPI_FIRMWARE_URL:=@GITHUB/raspberrypi/firmware/$(PKG_RELEASE)/boot/
RPI_FIRMWARE_FILE:=rpi-firmware-$(PKG_RELEASE)

define Download/LICENCE_broadcom
  FILE:=$(RPI_FIRMWARE_FILE)-LICENCE.broadcom
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=LICENCE.broadcom
  HASH:=ba76edfc10a248166d965b8eaf320771c44f4f432d4fce2fd31fd272e7038add
endef
$(eval $(call Download,LICENCE_broadcom))

define Download/bootcode_bin
  FILE:=$(RPI_FIRMWARE_FILE)-bootcode.bin
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=bootcode.bin
  HASH:=7b24659eb049333eec69f59cf0c5aa0d49eab5ed67726af3c6f0c9bcf1e3f9e3
endef
$(eval $(call Download,bootcode_bin))

define Download/fixup_dat
  FILE:=$(RPI_FIRMWARE_FILE)-fixup.dat
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=fixup.dat
  HASH:=5e5946fe7c0b1f5e270f43a17aef5f6da5758d5e544ed37137628c972c9b8061
endef
$(eval $(call Download,fixup_dat))

define Download/fixup_cd_dat
  FILE:=$(RPI_FIRMWARE_FILE)-fixup_cd.dat
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=fixup_cd.dat
  HASH:=d7ed04063af818695f6d2f6ffdde8304c19385051c9cee7f8d6d5b20851e4e55
endef
$(eval $(call Download,fixup_cd_dat))

define Download/fixup_x_dat
  FILE:=$(RPI_FIRMWARE_FILE)-fixup_x.dat
  URL:=$(RPI_FIRMWARE_URL)
  HASH:=ec99af78227d9ffa2e46ab80a96aa762187467fe33876e4d8085377e7d0f76c2
  URL_FILE:=fixup_x.dat
endef
$(eval $(call Download,fixup_x_dat))

define Download/start_elf
  FILE:=$(RPI_FIRMWARE_FILE)-start.elf
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=start.elf
  HASH:=7e69d068c249cd859f5c44a8cf80a5e96a3f3fbeeeb0260de7373130a1d9b0fe
endef
$(eval $(call Download,start_elf))

define Download/start_cd_elf
  FILE:=$(RPI_FIRMWARE_FILE)-start_cd.elf
  URL:=$(RPI_FIRMWARE_URL)
  URL_FILE:=start_cd.elf
  HASH:=08aee2119a19c3556099ff875181d8e5440ebd1cecf1a5936b9340fe41da4428
endef
$(eval $(call Download,start_cd_elf))

define Download/start_x_elf
  FILE:=$(RPI_FIRMWARE_FILE)-start_x.elf
  URL:=$(RPI_FIRMWARE_URL)
  HASH:=4fa98ee8eca7427aa013c1ad4a70d7b4cd01dcc08264307d1cb62f7c9ddfeed9
  URL_FILE:=start_x.elf
endef
$(eval $(call Download,start_x_elf))

define Package/brcm2708-gpu-fw
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_brcm2708
  TITLE:=brcm2708-gpu-fw
  DEFAULT:=y if TARGET_brcm2708
endef

define Package/brcm2708-gpu-fw/description
 GPU and kernel boot firmware for brcm2708. 
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-LICENCE.broadcom $(PKG_BUILD_DIR)/LICENCE.broadcom
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-bootcode.bin $(PKG_BUILD_DIR)/bootcode.bin
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-fixup.dat $(PKG_BUILD_DIR)/fixup.dat
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-fixup_cd.dat $(PKG_BUILD_DIR)/fixup_cd.dat
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-fixup_x.dat $(PKG_BUILD_DIR)/fixup_x.dat
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-start.elf $(PKG_BUILD_DIR)/start.elf
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-start_cd.elf $(PKG_BUILD_DIR)/start_cd.elf
	cp $(DL_DIR)/$(RPI_FIRMWARE_FILE)-start_x.elf $(PKG_BUILD_DIR)/start_x.elf
endef

define Build/Compile
	true
endef

define Package/brcm2708-gpu-fw/install
	true
endef

define Build/InstallDev
	$(CP) $(PKG_BUILD_DIR)/bootcode.bin $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/LICENCE.broadcom $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/start.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/start_cd.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/start_x.elf $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/fixup.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/fixup_cd.dat $(KERNEL_BUILD_DIR)
	$(CP) $(PKG_BUILD_DIR)/fixup_x.dat $(KERNEL_BUILD_DIR)
endef

$(eval $(call BuildPackage,brcm2708-gpu-fw))
