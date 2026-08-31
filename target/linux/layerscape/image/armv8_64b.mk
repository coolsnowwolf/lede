# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright 2018-2020 NXP

define Device/Default
  PROFILES := Default
  IMAGES := firmware.bin sysupgrade.bin
  DEVICE_DTS_DIR := $(DTS_DIR)/freescale
  DEVICE_DTS = $(subst _,-,$(1))
  FILESYSTEMS := squashfs
  KERNEL := kernel-bin | gzip | uImage gzip
  KERNEL_INITRAMFS = kernel-bin | gzip | fit gzip $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_LOADADDR := 0x80000000
  IMAGE_SIZE := 64m
  IMAGE/sysupgrade.bin = \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 1M | \
    append-kernel | pad-to 17M | \
    append-rootfs | pad-rootfs | \
    check-size $(LS_SYSUPGRADE_IMAGE_SIZE) | append-metadata
endef

define Device/fsl-sdboot
  KERNEL = kernel-bin | gzip | fit gzip $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGES := sdcard.img.gz sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/fsl_ls1012a-frdm
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRDM-LS1012A
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    kmod-ppfe
  BLOCKSIZE := 256KiB
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 10M | \
    ls-append pfe.itb | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to $$(BLOCKSIZE) | \
    append-rootfs | pad-rootfs | check-size
  IMAGE/sysupgrade.bin := \
    append-kernel | pad-to $$(BLOCKSIZE) | \
    append-rootfs | pad-rootfs | \
    check-size $(LS_SYSUPGRADE_IMAGE_SIZE) | append-metadata
  KERNEL := kernel-bin | gzip | fit gzip $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
endef
TARGET_DEVICES += fsl_ls1012a-frdm

define Device/fsl_ls1012a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1012A-RDB
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    kmod-hwmon-ina2xx \
    kmod-iio-fxas21002c-i2c \
    kmod-iio-fxos8700-i2c \
    kmod-ppfe
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 10M | \
    ls-append pfe.itb | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1012a-rdb

define Device/fsl_ls1012a-frwy-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRWY-LS1012A
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    kmod-ppfe
  DEVICE_DTS := fsl-ls1012a-frwy
  IMAGES += firmware.bin
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 128K | \
    ls-append pfe.itb | pad-to 384K | \
    ls-append $(1)-fip.bin | pad-to 1856K | \
    ls-append $(1)-uboot-env.bin | pad-to 2048K | \
    check-size 2097153
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1012a-frwy-sdboot

define Device/fsl_ls1028a-rdb
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1028A-RDB
  DEVICE_VARIANT := Default
  KERNEL = kernel-bin | gzip | fit gzip $$(DEVICE_DTS_DIR)/$$(DEVICE_DTS).dtb
  DEVICE_PACKAGES += \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90 \
    kmod-rtc-pcf2127
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 16M | \
    append-kernel | \
    append-rootfs | pad-rootfs | check-size
  IMAGE/sysupgrade.bin := \
    append-kernel | \
    append-rootfs | pad-rootfs | \
    check-size $(LS_SYSUPGRADE_IMAGE_SIZE) | append-metadata
endef
TARGET_DEVICES += fsl_ls1028a-rdb

define Device/fsl_ls1028a-rdb-sdboot
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1028A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := fsl-ls1028a-rdb
  DEVICE_PACKAGES += \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90 \
    kmod-rtc-pcf2127
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1028a-rdb-sdboot

define Device/fsl_ls1043a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1043A-RDB
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append $(1)-fman.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1043a-rdb

define Device/fsl_ls1043a-rdb-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1043A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := fsl-ls1043a-rdb
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append fsl_ls1043a-rdb-fman.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1043a-rdb-sdboot

define Device/fsl_ls1046a-frwy
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRWY-LS1046A
  DEVICE_VARIANT := Default
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append fsl_ls1046a-rdb-fman.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1046a-frwy

define Device/fsl_ls1046a-frwy-sdboot
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRWY-LS1046A
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := fsl-ls1046a-frwy
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append fsl_ls1046a-rdb-fman.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1046a-frwy-sdboot

define Device/fsl_ls1046a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1046A-RDB
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append $(1)-fman.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1046a-rdb

define Device/fsl_ls1046a-rdb-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1046A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := fsl-ls1046a-rdb
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 9M | \
    ls-append fsl_ls1046a-rdb-fman.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1046a-rdb-sdboot

define Device/fsl_ls1088a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1088A-RDB
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += \
    restool \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 10M | \
    ls-append $(1)-mc.itb | pad-to 13M | \
    ls-append $(1)-dpl.dtb | pad-to 14M | \
    ls-append $(1)-dpc.dtb | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1088a-rdb

define Device/fsl_ls1088a-rdb-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1088A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    restool \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := fsl-ls1088a-rdb
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 10M | \
    ls-append fsl_ls1088a-rdb-mc.itb | pad-to 13M | \
    ls-append fsl_ls1088a-rdb-dpl.dtb | pad-to 14M | \
    ls-append fsl_ls1088a-rdb-dpc.dtb | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1088a-rdb-sdboot

define Device/fsl_ls2088a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS2088ARDB
  DEVICE_PACKAGES += \
    restool \
    kmod-ahci-qoriq
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 10M | \
    ls-append $(1)-mc.itb | pad-to 13M | \
    ls-append $(1)-dpl.dtb | pad-to 14M | \
    ls-append $(1)-dpc.dtb | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls2088a-rdb

define Device/fsl_lx2160a-rdb
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LX2160A-RDB
  DEVICE_VARIANT := Rev2.0 silicon
  DEVICE_PACKAGES += restool
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 8M | \
    ls-append $(1)-fip_ddr_all.bin | pad-to 10M | \
    ls-append $(1)-mc.itb | pad-to 13M | \
    ls-append $(1)-dpl.dtb | pad-to 14M | \
    ls-append $(1)-dpc.dtb | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_lx2160a-rdb

define Device/fsl_lx2160a-rdb-sdboot
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LX2160A-RDB
  DEVICE_VARIANT := Rev2.0 silicon SD Card Boot
  DEVICE_PACKAGES += restool
  DEVICE_DTS := fsl-lx2160a-rdb
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-bl2.pbl | pad-to 1M | \
    ls-append $(1)-fip.bin | pad-to 5M | \
    ls-append $(1)-uboot-env.bin | pad-to 8M | \
    ls-append fsl_lx2160a-rdb-fip_ddr_all.bin | pad-to 10M | \
    ls-append fsl_lx2160a-rdb-mc.itb | pad-to 13M | \
    ls-append fsl_lx2160a-rdb-dpl.dtb | pad-to 14M | \
    ls-append fsl_lx2160a-rdb-dpc.dtb | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_lx2160a-rdb-sdboot

define Device/traverse_ten64-mtd
  DEVICE_VENDOR := Traverse
  DEVICE_MODEL := Ten64 (NAND boot)
  BOARD_NAME := ten64-mtd
  DEVICE_PACKAGES += \
    uboot-envtools \
    kmod-rtc-rx8025 \
    kmod-sfp \
    kmod-i2c-mux-pca954x \
    restool
  DEVICE_DESCRIPTION = \
    Generate images for booting from NAND/ubifs on Traverse Ten64 (LS1088A) \
    family boards. For disk (NVMe/USB/SD) boot, use the armvirt target instead.
  FILESYSTEMS := squashfs
  KERNEL_LOADADDR := 0x80000000
  KERNEL_ENTRY_POINT := 0x80000000
  FDT_LOADADDR := 0x90000000
  KERNEL_SUFFIX := -kernel.itb
  DEVICE_DTS := fsl-ls1088a-ten64
  IMAGES := nand.ubi sysupgrade.bin
  KERNEL := kernel-bin | gzip | traverse-fit-ls1088 gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb $$(FDT_LOADADDR)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/nand.ubi := append-ubi
  KERNEL_IN_UBI := 1
  BLOCKSIZE := 128KiB
  PAGESIZE := 2048
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB -c 600
  SUPPORTED_DEVICES = traverse,ten64
endef
TARGET_DEVICES += traverse_ten64-mtd

