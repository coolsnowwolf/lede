#
# Copyright 2018 NXP
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Device/Default
  PROFILES := Default
  FILESYSTEMS := squashfs
  IMAGES := firmware.bin
  KERNEL := kernel-bin | uImage none
  KERNEL_NAME := zImage
  KERNEL_LOADADDR := 0x80008000
  KERNEL_ENTRY_POINT := 0x80008000
endef

define Device/ls1021atwr
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := TWR-LS1021A
  DEVICE_VARIANT := Default
  DEVICE_PACKAGES += layerscape-rcw
  DEVICE_DTS := ls1021a-twr
  IMAGE/firmware.bin := \
    ls-clean | \
    ls-append $(1)-rcw.bin | pad-to 1M | \
    ls-append $(1)-uboot.bin | pad-to 3M | \
    ls-append $(1)-uboot-env.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to 32M | \
    append-rootfs | pad-rootfs | check-size 67108865
endef
TARGET_DEVICES += ls1021atwr

define Device/ls1021atwr-sdboot
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := TWR-LS1021A
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := ls1021a-twr
  FILESYSTEMS := ext4
  IMAGES := sdcard.img
  IMAGE/sdcard.img := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-uboot.bin | pad-to 3M | \
    ls-append $(1)-uboot-env.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | check-size $(LS_SD_IMAGE_SIZE)
endef
TARGET_DEVICES += ls1021atwr-sdboot

define Device/ls1021aiot-sdboot
  DEVICE_VENDOR := NXP
  DEVICE_MODEL := LS1021A-IoT
  DEVICE_VARIANT := SD Card Boot
  DEVICE_DTS := ls1021a-iot
  FILESYSTEMS := ext4
  IMAGES := sdcard.img
  IMAGE/sdcard.img := \
    ls-clean | \
    ls-append-sdhead $(1) | pad-to 4K | \
    ls-append $(1)-uboot.bin | pad-to 1M | \
    ls-append $(1)-uboot-env.bin | pad-to 15M | \
    ls-append-dtb $$(DEVICE_DTS) | pad-to 16M | \
    append-kernel | pad-to $(LS_SD_ROOTFSPART_OFFSET)M | \
    append-rootfs | check-size $(LS_SD_IMAGE_SIZE)
endef
TARGET_DEVICES += ls1021aiot-sdboot
