#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(SUBTARGET),cortexa9)

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
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
endef
TARGET_DEVICES += linksys-wrt1200ac

define Device/linksys-wrt1900acv2
  $(call Device/linksys,WRT1900ACv2 (Cobra))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-cobra
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
endef
TARGET_DEVICES += linksys-wrt1900acv2

define Device/linksys-wrt3200acm
  $(call Device/linksys,WRT3200ACM (Rango))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-rango
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio mwlwifi-firmware-88w8964
endef
TARGET_DEVICES += linksys-wrt3200acm

define Device/linksys-wrt1900acs
  $(call Device/linksys,WRT1900ACS (Shelby))
  $(Device/armada-385-linksys)
  DEVICE_DTS := armada-385-linksys-shelby
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
endef
TARGET_DEVICES += linksys-wrt1900acs

define Device/linksys-wrt1900ac
  $(call Device/linksys,WRT1900AC (Mamba))
  DEVICE_DTS := armada-xp-linksys-mamba
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
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
  IMAGE/sdcard.img.gz := boot-scr | boot-img | sdcard-img | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
  SUPPORTED_DEVICES := armada-388-clearfog-pro armada-388-clearfog
  UBOOT := clearfog-u-boot-spl.kwb
endef
TARGET_DEVICES += armada-388-clearfog-pro

define Device/armada-388-clearfog-base
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_TITLE := SolidRun ClearFog Base
  DEVICE_PACKAGES := mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img | sdcard-img | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
  UBOOT := clearfog-u-boot-spl.kwb
endef
TARGET_DEVICES += armada-388-clearfog-base

define Device/globalscale-mirabox
  $(Device/NAND-512K)
  DEVICE_DTS := armada-370-mirabox
  SUPPORTED_DEVICES := mirabox
  DEVICE_TITLE := Globalscale Mirabox
endef
TARGET_DEVICES += globalscale-mirabox

define Device/turris-omnia
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  KERNEL_INITRAMFS := kernel-bin
  DEVICE_TITLE := Turris Omnia
  DEVICE_PACKAGES :=  \
    mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 \
    wpad-mini kmod-ath9k kmod-ath10k ath10k-firmware-qca988x
  IMAGES := $$(IMAGE_PREFIX)-sysupgrade.img.gz omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz
  IMAGE/$$(IMAGE_PREFIX)-sysupgrade.img.gz := boot-img | sdcard-img | gzip | append-metadata
  IMAGE/omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz := omnia-medkit-initramfs | gzip
  IMAGE_NAME = $$(2)
  DEVICE_DTS := armada-385-turris-omnia
endef
TARGET_DEVICES += turris-omnia

endif
