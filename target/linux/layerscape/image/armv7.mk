# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright 2018-2020 NXP

define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs
  IMAGES := firmware.bin sysupgrade.bin
  KERNEL := kernel-bin | uImage none
  KERNEL_INITRAMFS = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  KERNEL_NAME := zImage
  KERNEL_LOADADDR := 0x80008000
  KERNEL_ENTRY_POINT := 0x80008000
  DEVICE_DTS = $(lastword $(subst _, ,$(1)))
  IMAGE_SIZE := 64m
  IMAGE/sysupgrade.bin = \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 1M | \
    append-kernel | pad-to 17M | \
    append-rootfs | pad-rootfs | \
    check-size $(LS_SYSUPGRADE_IMAGE_SIZE) | append-metadata
endef

define Device/fsl-sdboot
  KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
  IMAGES := sdcard.img.gz sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/fsl_ls1021a-twr
  $(Device/fix-sysupgrade)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := TWR-LS1021A
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += layerscape-rcw
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-rcw.bin | pad-to 1M | \
    ls-append $(1)-uboot.bin | pad-to 3M | \
    ls-append $(1)-uboot-env.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size
endef
TARGET_DEVICES += fsl_ls1021a-twr

define Device/fsl_ls1021a-twr-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := TWR-LS1021A
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := ls1021a-twr
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-uboot.bin | pad-to 3M | \
    ls-append $(1)-uboot-env.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1021a-twr-sdboot

define Device/fsl_ls1021a-iot-sdboot
  $(Device/rework-sdcard-images)
  $(Device/fsl-sdboot)
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1021A-IoT
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := ls1021a-iot
  SUPPORTED_DEVICES :=
  IMAGE/sdcard.img.gz := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-uboot.bin | pad-to 1M | \
    ls-append $(1)-uboot-env.bin | pad-to 16M | \
    ls-append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | pad-to $(LS_SD_IMAGE_SIZE)M | gzip
endef
TARGET_DEVICES += fsl_ls1021a-iot-sdboot
