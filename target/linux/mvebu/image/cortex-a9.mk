#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(SUBTARGET),cortexa9)

define Device/linksys
  $(Device/NAND-128K)
  DEVICE_TITLE := Linksys $(1)
  DEVICE_PACKAGES := kmod-mwlwifi swconfig wpad-basic
  IMAGES += factory.img
  KERNEL_SIZE := 6144k
endef

define Device/linksys_wrt1200ac
  $(call Device/linksys,WRT1200AC (Caiman))
  DEVICE_DTS := armada-385-linksys-caiman
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES := armada-385-linksys-caiman linksys,caiman
endef
TARGET_DEVICES += linksys_wrt1200ac

define Device/linksys_wrt1900acv2
  $(call Device/linksys,WRT1900ACv2 (Cobra))
  DEVICE_DTS := armada-385-linksys-cobra
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES := armada-385-linksys-cobra linksys,cobra
endef
TARGET_DEVICES += linksys_wrt1900acv2

define Device/linksys_wrt3200acm
  $(call Device/linksys,WRT3200ACM (Rango))
  DEVICE_DTS := armada-385-linksys-rango
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio mwlwifi-firmware-88w8964
  SUPPORTED_DEVICES := armada-385-linksys-rango linksys,rango
endef
TARGET_DEVICES += linksys_wrt3200acm

define Device/linksys_wrt1900acs
  $(call Device/linksys,WRT1900ACS (Shelby))
  DEVICE_DTS := armada-385-linksys-shelby
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES := armada-385-linksys-shelby linksys,shelby
endef
TARGET_DEVICES += linksys_wrt1900acs

define Device/linksys_wrt32x
  $(call Device/linksys,WRT32X (Venom))
  DEVICE_DTS := armada-385-linksys-venom
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio mwlwifi-firmware-88w8964
  KERNEL_SIZE := 3072k
  KERNEL := kernel-bin | append-dtb
  SUPPORTED_DEVICES := armada-385-linksys-venom linksys,venom
endef
TARGET_DEVICES += linksys_wrt32x

define Device/linksys_wrt1900ac
  $(call Device/linksys,WRT1900AC (Mamba))
  DEVICE_DTS := armada-xp-linksys-mamba
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  KERNEL_SIZE := 3072k
  SUPPORTED_DEVICES := armada-xp-linksys-mamba linksys,mamba
endef
TARGET_DEVICES += linksys_wrt1900ac

define Device/plathome_openblocks-ax3-4
  DEVICE_DTS := armada-xp-openblocks-ax3-4
  SUPPORTED_DEVICES += openblocks-ax3-4
  BLOCKSIZE := 128k
  PAGESIZE := 1
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$(BLOCKSIZE) | append-ubi
  DEVICE_TITLE := Plat'Home OpenBlocks AX3
endef
TARGET_DEVICES += plathome_openblocks-ax3-4

define Device/marvell_a385-db-ap
  $(Device/NAND-256K)
  DEVICE_DTS := armada-385-db-ap
  IMAGES += factory.img
  KERNEL_SIZE := 8192k
  DEVICE_TITLE := Marvell Armada 385 DB AP (DB-88F6820-AP)
  SUPPORTED_DEVICES += armada-385-db-ap
endef
TARGET_DEVICES += marvell_a385-db-ap

define Device/marvell_a370-db
  $(Device/NAND-512K)
  DEVICE_TITLE := Marvell Armada 370 DB (DB-88F6710-BP-DDR3)
  DEVICE_DTS := armada-370-db
  SUPPORTED_DEVICES += armada-370-db
endef
TARGET_DEVICES += marvell_a370-db

define Device/marvell_a370-rd
  $(Device/NAND-512K)
  DEVICE_TITLE := Marvell Armada 370 RD (RD-88F6710-A1)
  DEVICE_DTS := armada-370-rd
  SUPPORTED_DEVICES += armada-370-rd
endef
TARGET_DEVICES += marvell_a370-rd

define Device/marvell_axp-db
  $(Device/NAND-512K)
  DEVICE_TITLE := Marvell Armada XP DB (DB-78460-BP)
  DEVICE_DTS := armada-xp-db
  SUPPORTED_DEVICES += armada-xp-db
endef
TARGET_DEVICES += marvell_axp-db

define Device/marvell_axp-gp
  $(Device/NAND-512K)
  DEVICE_TITLE := Marvell Armada XP GP (DB-MV784MP-GP)
  DEVICE_DTS := armada-xp-gp
  SUPPORTED_DEVICES += armada-xp-gp
endef
TARGET_DEVICES += marvell_axp-gp

define Device/marvell_a388-rd
  DEVICE_TITLE := Marvell Armada 388 RD (RD-88F6820-AP)
  DEVICE_DTS := armada-388-rd
  IMAGES := firmware.bin
  IMAGE/firmware.bin := append-kernel | pad-to 256k | append-rootfs | pad-rootfs
  SUPPORTED_DEVICES := armada-388-rd marvell,a385-rd
endef
TARGET_DEVICES += marvell_a388-rd

define Device/solidrun_clearfog-pro-a1
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_TITLE := SolidRun ClearFog Pro
  DEVICE_PACKAGES := mkf2fs e2fsprogs partx-utils swconfig
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
  SUPPORTED_DEVICES += armada-388-clearfog armada-388-clearfog-pro
  UBOOT := clearfog-u-boot-spl.kwb
  BOOT_SCRIPT := clearfog
endef
TARGET_DEVICES += solidrun_clearfog-pro-a1

define Device/solidrun_clearfog-base-a1
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_TITLE := SolidRun ClearFog Base
  DEVICE_PACKAGES := mkf2fs e2fsprogs partx-utils
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-base armada-388-clearfog-pro
  SUPPORTED_DEVICES += armada-388-clearfog-base
  UBOOT := clearfog-u-boot-spl.kwb
  BOOT_SCRIPT := clearfog
endef
TARGET_DEVICES += solidrun_clearfog-base-a1

define Device/globalscale_mirabox
  $(Device/NAND-512K)
  DEVICE_DTS := armada-370-mirabox
  SUPPORTED_DEVICES += mirabox
  DEVICE_TITLE := Globalscale Mirabox
endef
TARGET_DEVICES += globalscale_mirabox

define Device/cznic_turris-omnia
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  KERNEL_INITRAMFS := kernel-bin
  DEVICE_TITLE := Turris Omnia
  DEVICE_PACKAGES :=  \
    mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 \
    wpad-basic kmod-ath9k kmod-ath10k-ct ath10k-firmware-qca988x-ct \
    partx-utils kmod-i2c-core kmod-i2c-mux kmod-i2c-mux-pca954x
  IMAGES := $$(IMAGE_PREFIX)-sysupgrade.img.gz omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz
  IMAGE/$$(IMAGE_PREFIX)-sysupgrade.img.gz := boot-img | sdcard-img | gzip | append-metadata
  IMAGE/omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz := omnia-medkit-initramfs | gzip
  IMAGE_NAME = $$(2)
  DEVICE_DTS := armada-385-turris-omnia
  SUPPORTED_DEVICES += armada-385-turris-omnia
endef
TARGET_DEVICES += cznic_turris-omnia

endif
