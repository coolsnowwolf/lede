#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

JFFS2_BLOCKSIZE = 128k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x00008000

define Build/dtb
        $(call Image/BuildDTB,$(DTS_DIR)/$(DEVICE_DTS).dts,$@.dtb)
endef

# SD-Card Images:
# these values are optimized for a 4GB labeled sdcard that actually holds 7744512 sectors of 512 byte
# MBR:            2048 sectors
# Partition 1:   32768 sectors
# Partition 2:   98304 sectors (configurable)

define Build/boot-scr-cfpro
	rm -f $@.bootscript
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d cfpro-boot.script $@.bootscript
endef

define Build/boot-scr-cfbase
	rm -f $@.bootscript
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d cfbase-boot.script $@.bootscript
endef

define Build/boot-img
	rm -f $@.boot
	mkfs.fat -C $@.boot 16384
	$(foreach dts,$(DEVICE_DTS), mcopy -i $@.boot $(DTS_DIR)/$(dts).dtb ::$(dts).dtb;)
	mcopy -i $@.boot $(IMAGE_KERNEL) ::zImage
	mcopy -i $@.boot $@.bootscript ::boot.scr
endef

define Build/sdcard-img
	ROOTFS_SIZE=$$(( $(CONFIG_TARGET_ROOTFS_PARTSIZE) * 1024 * 2 )); \
	./gen_mvebu_sdcard_img.sh $@ \
		"$(STAGING_DIR_IMAGE)/clearfog-u-boot-spl.kwb" \
		c 32768 $@.boot \
		83 $$ROOTFS_SIZE $(IMAGE_ROOTFS)
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS := $(1)
  BOARD_NAME = $$(DEVICE_DTS)
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  SUPPORTED_DEVICES = $$(DEVICE_DTS)
endef

define Device/UBI
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/UBI-factory
  $(Device/UBI)
  UBINIZE_OPTS := -E 5
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | pad-to $$$$(PAGESIZE)
endef

define Device/NAND-128K
  $(Device/UBI)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  VID_HDR_OFFSET := 2048
endef

define Device/NAND-256K
  $(Device/UBI)
  BLOCKSIZE := 256k
  PAGESIZE := 4096
endef

define Device/NAND-512K
  $(Device/UBI)
  BLOCKSIZE := 512k
  PAGESIZE := 4096
endef

define Device/linksys
  DEVICE_TITLE := Linksys $(1)
  DEVICE_PACKAGES := kmod-mwlwifi wpad-mini swconfig
endef

define Device/armada-385-linksys
  $(Device/NAND-128K)
  $(Device/UBI-factory)
  KERNEL_SIZE := 6144k
endef

define Device/linksys-wrt1200ac
  $(call Device/linksys,WRT1200AC (Caiman))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-caiman
endef
TARGET_DEVICES += linksys-wrt1200ac

define Device/linksys-wrt1900acv2
  $(call Device/linksys,WRT1900ACv2 (Cobra))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-cobra
endef
TARGET_DEVICES += linksys-wrt1900acv2

define Device/linksys-wrt3200acm
  $(call Device/linksys,WRT3200ACM (Rango))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-rango
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio
endef
TARGET_DEVICES += linksys-wrt3200acm

define Device/linksys-wrt1900acs
  $(call Device/linksys,WRT1900ACS (Shelby))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-shelby
endef
TARGET_DEVICES += linksys-wrt1900acs

define Device/linksys-wrt1900ac
  $(call Device/linksys,WRT1900AC (Mamba))
  DEVICE_DTS := armada-xp-linksys-mamba
  $(Device/NAND-128K)
  $(Device/UBI-factory)
  KERNEL_SIZE := 3072k
endef
TARGET_DEVICES += linksys-wrt1900ac

define Device/openblocks-ax3-4
  $(Device/UBI-factory)
  DEVICE_DTS := armada-xp-openblocks-ax3-4
  SUPPORTED_DEVICES := $(1)
  BLOCKSIZE := 128k
  PAGESIZE := 1
  IMAGE/factory.img := append-kernel | pad-to $$(BLOCKSIZE) | append-ubi
  DEVICE_TITLE := Plat'Home OpenBlocks AX3
endef
TARGET_DEVICES += openblocks-ax3-4

define Device/armada-385-db-ap
  $(Device/NAND-256K)
  $(Device/UBI-factory)
  KERNEL_SIZE := 8192k
  DEVICE_TITLE := Marvell Armada 385 DB AP (DB-88F6820-AP)
endef
TARGET_DEVICES += armada-385-db-ap

define Device/marvell-nand
  $(Device/NAND-512K)
  DEVICE_TITLE := Marvell Armada $(1)
endef

define Device/armada-370-db
	$(call Device/marvell-nand,370 DB (DB-88F6710-BP-DDR3))
endef
TARGET_DEVICES += armada-370-db

define Device/armada-370-rd
	$(call Device/marvell-nand,370 RD (RD-88F6710-A1))
endef
TARGET_DEVICES += armada-370-rd

define Device/armada-xp-db
	$(call Device/marvell-nand,XP DB (DB-78460-BP))
endef
TARGET_DEVICES += armada-xp-db

define Device/armada-xp-gp
	$(call Device/marvell-nand,XP GP (DB-MV784MP-GP))
endef
TARGET_DEVICES += armada-xp-gp

define Device/armada-388-rd
  DEVICE_TITLE := Marvell Armada 388 RD (RD-88F6820-AP)
  IMAGES := firmware.bin
  IMAGE/firmware.bin := append-kernel | pad-to 256k | append-rootfs | pad-rootfs
endef
TARGET_DEVICES += armada-388-rd

define Device/armada-388-clearfog-pro
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_TITLE := SolidRun ClearFog Pro
  DEVICE_PACKAGES := mkf2fs e2fsprogs swconfig kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr-cfpro | boot-img | sdcard-img | gzip | append-metadata
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(2)
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
endef
TARGET_DEVICES += armada-388-clearfog-pro

define Device/armada-388-clearfog-base
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_TITLE := SolidRun ClearFog Base
  DEVICE_PACKAGES := mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr-cfbase | boot-img | sdcard-img | gzip | append-metadata
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(2)
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
endef
TARGET_DEVICES += armada-388-clearfog-base

define Device/globalscale-mirabox
  $(Device/NAND-512K)
  DEVICE_DTS := armada-370-mirabox
  SUPPORTED_DEVICES := mirabox
  DEVICE_TITLE := Globalscale Mirabox
endef
TARGET_DEVICES += globalscale-mirabox

$(eval $(call BuildImage))
