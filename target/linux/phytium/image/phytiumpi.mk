# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2023 Phytium Technology Co., Ltd.

include dts_common.mk

define Device/phytiumpi_firefly_4GB
  DEVICE_VENDOR := FireFly
  DEVICE_MODEL := PhytiumPi 4GB
  SOC := phytiumpi
  DEVICE_DTS := phytiumpi_firefly
  KERNEL_SUFFIX := -fit-uImage.itb
  DEVICE_DTS_CONFIG := phytium
  KERNEL := kernel-bin | gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  UBOOT_PATH := ./bin/fip-all-4GB.bin
  FILESYSTEMS := ext4
  IMAGES += factory.bin
  SUPPORTED_DEVICES := phytiumpi_firefly
  IMAGE/factory.bin := append-uboot | pad-to 4096k | append-kernel | pad-to 10240k |\
	  append-rootfs | pad-rootfs | append-metadata
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += phytiumpi_firefly_4GB

define Device/phytiumpi_firefly_2GB
  DEVICE_VENDOR := FireFly
  DEVICE_MODEL := PhytiumPi 2GB
  SOC := phytiumpi
  DEVICE_DTS := phytiumpi_firefly
  KERNEL_SUFFIX := -fit-uImage.itb
  DEVICE_DTS_CONFIG := phytium
  KERNEL := kernel-bin | gzip | fit gzip $(KDIR)/image-$$(DEVICE_DTS).dtb
  UBOOT_PATH := ./bin/fip-all-2GB.bin
  FILESYSTEMS := ext4
  IMAGES += factory.bin
  SUPPORTED_DEVICES := phytiumpi_firefly
  IMAGE/factory.bin := append-uboot | pad-to 4096k | append-kernel | pad-to 10240k |\
	  append-rootfs | pad-rootfs | append-metadata
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += phytiumpi_firefly_2GB
