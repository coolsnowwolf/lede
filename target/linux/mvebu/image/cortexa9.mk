#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA \
	(early adopters with DSA already set up may just force-flash keeping existing config)
endef

define Device/buffalo_ls421de
  $(Device/NAND-128K)
  DEVICE_VENDOR := Buffalo
  DEVICE_MODEL := LinkStation LS421DE
  SUBPAGESIZE :=
  KERNEL_SIZE := 33554432
  FILESYSTEMS := squashfs ubifs
  KERNEL := kernel-bin | append-dtb | uImage none | buffalo-kernel-jffs2
  KERNEL_INITRAMFS := kernel-bin | append-dtb | uImage none
  DEVICE_DTS := armada-370-buffalo-ls421de
  DEVICE_PACKAGES :=  \
    kmod-rtc-rs5c372a kmod-hwmon-gpiofan kmod-hwmon-drivetemp kmod-usb3 \
    kmod-md-raid0 kmod-md-raid1 kmod-md-mod kmod-fs-xfs mkf2fs e2fsprogs \
    partx-utils
endef
TARGET_DEVICES += buffalo_ls421de

define Device/cznic_turris-omnia
  DEVICE_VENDOR := CZ.NIC
  DEVICE_MODEL := Turris Omnia
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  KERNEL_INITRAMFS := kernel-bin
  DEVICE_PACKAGES :=  \
    mkf2fs e2fsprogs kmod-fs-vfat kmod-nls-cp437 kmod-nls-iso8859-1 \
    wpad-basic-wolfssl kmod-ath9k kmod-ath10k-ct ath10k-firmware-qca988x-ct \
    partx-utils kmod-i2c-mux-pca954x
  IMAGES := $$(IMAGE_PREFIX)-sysupgrade.img.gz omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz
  IMAGE/$$(IMAGE_PREFIX)-sysupgrade.img.gz := boot-img | sdcard-img | gzip | append-metadata
  IMAGE/omnia-medkit-$$(IMAGE_PREFIX)-initramfs.tar.gz := omnia-medkit-initramfs | gzip
  IMAGE_NAME = $$(2)
  SOC := armada-385
  SUPPORTED_DEVICES += armada-385-turris-omnia
endef
TARGET_DEVICES += cznic_turris-omnia

define Device/globalscale_mirabox
  $(Device/NAND-512K)
  DEVICE_VENDOR := Globalscale
  DEVICE_MODEL := Mirabox
  SOC := armada-370
  SUPPORTED_DEVICES += mirabox
endef
TARGET_DEVICES += globalscale_mirabox

define Device/kobol_helios4
  DEVICE_VENDOR := Kobol
  DEVICE_MODEL := Helios4
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_PACKAGES := mkf2fs e2fsprogs partx-utils
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  SOC := armada-388
  UBOOT := helios4-u-boot-spl.kwb
  BOOT_SCRIPT := clearfog
endef
TARGET_DEVICES += kobol_helios4

define Device/linksys
  $(Device/NAND-128K)
  DEVICE_VENDOR := Linksys
  DEVICE_PACKAGES := kmod-mwlwifi wpad-basic-wolfssl
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
	append-ubi | pad-to $$$$(PAGESIZE)
  KERNEL_SIZE := 6144k
endef

define Device/linksys_wrt1200ac
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT1200AC
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := Caiman
  DEVICE_DTS := armada-385-linksys-caiman
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES += armada-385-linksys-caiman linksys,caiman
endef
TARGET_DEVICES += linksys_wrt1200ac

define Device/linksys_wrt1900acs
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT1900ACS
  DEVICE_VARIANT := v1
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := WRT1900ACS
  DEVICE_ALT0_VARIANT := v2
  DEVICE_ALT1_VENDOR := Linksys
  DEVICE_ALT1_MODEL := Shelby
  DEVICE_DTS := armada-385-linksys-shelby
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES += armada-385-linksys-shelby linksys,shelby
endef
TARGET_DEVICES += linksys_wrt1900acs

define Device/linksys_wrt1900ac-v1
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT1900AC
  DEVICE_VARIANT := v1
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := Mamba
  DEVICE_DTS := armada-xp-linksys-mamba
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  KERNEL_SIZE := 3072k
  SUPPORTED_DEVICES += armada-xp-linksys-mamba linksys,mamba
  DEFAULT := n
endef
TARGET_DEVICES += linksys_wrt1900ac-v1

define Device/linksys_wrt1900ac-v2
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT1900AC
  DEVICE_VARIANT := v2
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := Cobra
  DEVICE_DTS := armada-385-linksys-cobra
  DEVICE_PACKAGES += mwlwifi-firmware-88w8864
  SUPPORTED_DEVICES += armada-385-linksys-cobra linksys,cobra
endef
TARGET_DEVICES += linksys_wrt1900ac-v2

define Device/linksys_wrt3200acm
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT3200ACM
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := Rango
  DEVICE_DTS := armada-385-linksys-rango
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio mwlwifi-firmware-88w8964
  SUPPORTED_DEVICES += armada-385-linksys-rango linksys,rango
endef
TARGET_DEVICES += linksys_wrt3200acm

define Device/linksys_wrt32x
  $(call Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := WRT32X
  DEVICE_ALT0_VENDOR := Linksys
  DEVICE_ALT0_MODEL := Venom
  DEVICE_DTS := armada-385-linksys-venom
  DEVICE_PACKAGES += kmod-btmrvl kmod-mwifiex-sdio mwlwifi-firmware-88w8964
  KERNEL_SIZE := 3072k
  KERNEL := kernel-bin | append-dtb
  SUPPORTED_DEVICES += armada-385-linksys-venom linksys,venom
  DEFAULT := n
endef
TARGET_DEVICES += linksys_wrt32x

define Device/marvell_a370-db
  $(Device/NAND-512K)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 370 Development Board (DB-88F6710-BP-DDR3)
  DEVICE_DTS := armada-370-db
  SUPPORTED_DEVICES += armada-370-db
endef
TARGET_DEVICES += marvell_a370-db

define Device/marvell_a370-rd
  $(Device/NAND-512K)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 370 RD (RD-88F6710-A1)
  DEVICE_DTS := armada-370-rd
  SUPPORTED_DEVICES += armada-370-rd
endef
TARGET_DEVICES += marvell_a370-rd

define Device/marvell_a385-db-ap
  $(Device/NAND-256K)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 385 Development Board AP (DB-88F6820-AP)
  DEVICE_DTS := armada-385-db-ap
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | \
	append-ubi | pad-to $$$$(PAGESIZE)
  KERNEL_SIZE := 8192k
  SUPPORTED_DEVICES += armada-385-db-ap
endef
TARGET_DEVICES += marvell_a385-db-ap

define Device/marvell_a388-rd
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada 388 RD (RD-88F6820-AP)
  DEVICE_DTS := armada-388-rd
  IMAGES := firmware.bin
  IMAGE/firmware.bin := append-kernel | pad-to 256k | append-rootfs | pad-rootfs
  SUPPORTED_DEVICES := armada-388-rd marvell,a385-rd
endef
TARGET_DEVICES += marvell_a388-rd

define Device/marvell_axp-db
  $(Device/NAND-512K)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada XP Development Board (DB-78460-BP)
  DEVICE_DTS := armada-xp-db
  SUPPORTED_DEVICES += armada-xp-db
endef
TARGET_DEVICES += marvell_axp-db

define Device/marvell_axp-gp
  $(Device/NAND-512K)
  DEVICE_VENDOR := Marvell
  DEVICE_MODEL := Armada Armada XP GP (DB-MV784MP-GP)
  DEVICE_DTS := armada-xp-gp
  SUPPORTED_DEVICES += armada-xp-gp
endef
TARGET_DEVICES += marvell_axp-gp

define Device/plathome_openblocks-ax3-4
  DEVICE_VENDOR := Plat'Home
  DEVICE_MODEL := OpenBlocks AX3
  DEVICE_VARIANT := 4 ports
  SOC := armada-xp
  SUPPORTED_DEVICES += openblocks-ax3-4
  BLOCKSIZE := 128k
  PAGESIZE := 1
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$(BLOCKSIZE) | append-ubi
endef
TARGET_DEVICES += plathome_openblocks-ax3-4

define Device/solidrun_clearfog-base-a1
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := ClearFog Base
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_PACKAGES := mkf2fs e2fsprogs partx-utils
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-base armada-388-clearfog-pro
  UBOOT := clearfog-u-boot-spl.kwb
  BOOT_SCRIPT := clearfog
  SUPPORTED_DEVICES += armada-388-clearfog-base
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Ethernet interface rename has been dropped
endef
TARGET_DEVICES += solidrun_clearfog-base-a1

define Device/solidrun_clearfog-pro-a1
  $(Device/dsa-migration)
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := ClearFog Pro
  KERNEL_INSTALL := 1
  KERNEL := kernel-bin
  DEVICE_PACKAGES := mkf2fs e2fsprogs partx-utils
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  DEVICE_DTS := armada-388-clearfog-pro armada-388-clearfog-base
  UBOOT := clearfog-u-boot-spl.kwb
  BOOT_SCRIPT := clearfog
  SUPPORTED_DEVICES += armada-388-clearfog armada-388-clearfog-pro
endef
TARGET_DEVICES += solidrun_clearfog-pro-a1
