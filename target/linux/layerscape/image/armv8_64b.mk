#
# Copyright 2018-2020 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Device/Default
  PROFILES := Default
  IMAGES := firmware.bin sysupgrade.bin
  FILESYSTEMS := squashfs
  KERNEL := kernel-bin | gzip | uImage gzip
  KERNEL_INITRAMFS = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_LOADADDR := 0x80080000
  KERNEL_ENTRY_POINT := 0x80080000
  DEVICE_DTS = freescale/$(subst _,-,$(1))
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
  IMAGE_SIZE := 64m
  IMAGE/sysupgrade.bin = \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 1M | \
    append-kernel | pad-to 17M | \
    append-rootfs | pad-rootfs | \
    check-size $(LS_SYSUPGRADE_IMAGE_SIZE) | append-metadata
endef

define Device/fsl-sdboot
  $(Device/rework-sdcard-images)
  KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGES := sdcard.img.gz sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/fsl_ls1012a-frdm
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRDM-LS1012A
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    tfa-ls1012a-frdm \
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
  KERNEL := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_INITRAMFS := kernel-bin | fit none $$(DTS_DIR)/$$(DEVICE_DTS).dtb
endef
TARGET_DEVICES += fsl_ls1012a-frdm

define Device/fsl_ls1012a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1012A-RDB
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    tfa-ls1012a-rdb \
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
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := FRWY-LS1012A
  DEVICE_PACKAGES += \
    layerscape-ppfe \
    tfa-ls1012a-frwy-sdboot \
    kmod-ppfe
  DEVICE_DTS := freescale/fsl-ls1012a-frwy
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

define Device/fsl_ls1043a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1043A-RDB
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += \
    layerscape-fman \
    tfa-ls1043a-rdb \
    fmc fmc-eth-config \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := freescale/fsl-ls1043a-rdb-sdk
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
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1043A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    layerscape-fman \
    tfa-ls1043a-rdb-sdboot \
    fmc fmc-eth-config \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := freescale/fsl-ls1043a-rdb-sdk
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

define Device/fsl_ls1046a-rdb
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1046A-RDB
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += \
    layerscape-fman \
    tfa-ls1046a-rdb \
    fmc fmc-eth-config \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := freescale/fsl-ls1046a-rdb-sdk
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
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1046A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    layerscape-fman \
    tfa-ls1046a-rdb-sdboot \
    fmc fmc-eth-config \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := freescale/fsl-ls1046a-rdb-sdk
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
    layerscape-mc \
    layerscape-dpl \
    tfa-ls1088a-rdb \
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
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1088A-RDB
  DEVICE_VARIANT := SD Card Boot
  DEVICE_PACKAGES += \
    layerscape-mc \
    layerscape-dpl \
    tfa-ls1088a-rdb-sdboot \
    restool \
    kmod-ahci-qoriq \
    kmod-hwmon-ina2xx \
    kmod-hwmon-lm90
  DEVICE_DTS := freescale/fsl-ls1088a-rdb
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
    layerscape-mc \
    layerscape-dpl \
    tfa-ls2088a-rdb \
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

define Device/traverse_ls1043
  DEVICE_VENDOR := Traverse
  DEVICE_MODEL := LS1043 Boards
  KERNEL_NAME := Image
  KERNEL_SUFFIX := -kernel.itb
  KERNEL_INSTALL := 1
  FDT_LOADADDR = 0x90000000
  FILESYSTEMS := ubifs
  MKUBIFS_OPTS := -m 1 -e 262016 -c 128
  DEVICE_PACKAGES += \
    layerscape-fman \
    uboot-envtools \
    kmod-i2c-mux-pca954x \
    kmod-hwmon-core \
    kmod-gpio-pca953x kmod-input-gpio-keys-polled \
    kmod-rtc-isl1208
  DEVICE_DESCRIPTION = \
    Build images for Traverse LS1043 boards. This generates a single image \
    capable of booting on any of the boards in this family.
  DEVICE_DTS = freescale/traverse-ls1043s
  DEVICE_DTS_DIR = $(LINUX_DIR)/arch/arm64/boot/dts
  DEVICE_DTS_CONFIG = ls1043s
  KERNEL := kernel-bin | gzip | traverse-fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb $$(FDT_LOADADDR)
  KERNEL_INITRAMFS := kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb $$(FDT_LOADADDR)
  IMAGES = root sysupgrade.bin
  IMAGE/root = append-rootfs
  IMAGE/sysupgrade.bin = sysupgrade-tar | append-metadata
  MKUBIFS_OPTS := -m 2048 -e 124KiB -c 4096
  SUPPORTED_DEVICES := traverse,ls1043s traverse,ls1043v
endef
TARGET_DEVICES += traverse_ls1043
