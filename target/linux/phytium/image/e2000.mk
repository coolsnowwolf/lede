# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2023 Phytium Technology Co., Ltd.

include dts_common.mk

define Device/e2000q-demo-board
  DEVICE_VENDOR := Phytium
  DEVICE_MODEL := E2000Q Demo
  SOC := e2000
  UBOOT_PATH := ./bin/e2000q_demo_uboot.bin
  IMAGES += factory.bin
  IMAGE/factory.bin := append-uboot | pad-to 3072k | append-dtb |\
	  pad-to 3136k | append-kernel | pad-to 9280k |\
	  append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += e2000q-demo-board

define Device/e2000d-demo-board
  DEVICE_VENDOR := Phytium
  DEVICE_MODEL := E2000D Demo
  SOC := e2000
  UBOOT_PATH := ./bin/e2000d_demo_uboot.bin
  IMAGES += factory.bin
  IMAGE/factory.bin := append-uboot | pad-to 3072k | append-dtb |\
	  pad-to 3136k | append-kernel | pad-to 9280k |\
	  append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += e2000d-demo-board

define Device/e2000q-miniitx-board
  DEVICE_VENDOR := EmbedWay
  DEVICE_MODEL := E2000Q MiniITX
  SOC := e2000
  UBOOT_PATH := ./bin/e2000_miniitx_uboot.bin
  IMAGES += factory.bin
  IMAGE/factory.bin := append-uboot | pad-to 3072k | append-dtb |\
	  pad-to 3136k | append-kernel | pad-to 9280k |\
	  append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += e2000q-miniitx-board

define Device/e2000d-miniitx-board
  DEVICE_VENDOR := EmbedWay
  DEVICE_MODEL := E2000D MiniITX
  SOC := e2000
  UBOOT_PATH := ./bin/e2000_miniitx_uboot.bin
  IMAGES += factory.bin
  IMAGE/factory.bin := append-uboot | pad-to 3072k | append-dtb |\
	  pad-to 3136k | append-kernel | pad-to 9280k |\
	  append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += e2000d-miniitx-board
