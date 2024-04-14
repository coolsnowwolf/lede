# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2016 Yousong Zhou

KERNEL_LOADADDR:=0x40008000

define Device/sun50i
  SUNXI_DTS_DIR := allwinner/
  KERNEL_NAME := Image
  KERNEL := kernel-bin
endef

define Device/sun50i-a64
  SOC := sun50i-a64
  $(Device/sun50i)
endef

define Device/sun50i-h5
  SOC := sun50i-h5
  $(Device/sun50i)
endef

define Device/sun50i-h6
  SOC := sun50i-h6
  $(Device/sun50i)
endef

define Device/sun50i-h616
  SOC := sun50i-h616
  $(Device/sun50i)
endef

define Device/sun50i-h618
  SOC := sun50i-h618
  $(Device/sun50i)
endef

define Device/friendlyarm_nanopi-neo-plus2
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO Plus2
  SUPPORTED_DEVICES:=nanopi-neo-plus2
  $(Device/sun50i-h5)
endef
TARGET_DEVICES += friendlyarm_nanopi-neo-plus2

define Device/friendlyarm_nanopi-neo2
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO2
  SUPPORTED_DEVICES:=nanopi-neo2
  $(Device/sun50i-h5)
endef
TARGET_DEVICES += friendlyarm_nanopi-neo2

define Device/friendlyarm_nanopi-r1s-h5
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := Nanopi R1S H5
  DEVICE_PACKAGES := kmod-gpio-button-hotplug kmod-usb-net-rtl8152
  SUPPORTED_DEVICES:=nanopi-r1s-h5
  $(Device/sun50i-h5)
endef
TARGET_DEVICES += friendlyarm_nanopi-r1s-h5

define Device/libretech_all-h3-cc-h5
  DEVICE_VENDOR := Libre Computer
  DEVICE_MODEL := ALL-H3-CC
  DEVICE_VARIANT := H5
  $(Device/sun50i-h5)
  SUNXI_DTS := $$(SUNXI_DTS_DIR)$$(SOC)-libretech-all-h3-cc
endef
TARGET_DEVICES += libretech_all-h3-cc-h5

define Device/olimex_a64-olinuxino
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A64-Olinuxino
  DEVICE_PACKAGES := kmod-rtl8723bs rtl8723bu-firmware
  $(Device/sun50i-a64)
  SUNXI_DTS := $$(SUNXI_DTS_DIR)$$(SOC)-olinuxino
endef
TARGET_DEVICES += olimex_a64-olinuxino

define Device/olimex_a64-olinuxino-emmc
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A64-Olinuxino
  DEVICE_VARIANT := eMMC
  DEVICE_PACKAGES := kmod-rtl8723bs rtl8723bu-firmware
  $(Device/sun50i-a64)
  SUNXI_DTS := $$(SUNXI_DTS_DIR)$$(SOC)-olinuxino-emmc
endef
TARGET_DEVICES += olimex_a64-olinuxino-emmc

define Device/pine64_pine64-plus
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := Pine64+
  DEVICE_PACKAGES := kmod-rtl8723bs rtl8723bu-firmware
  $(Device/sun50i-a64)
endef
TARGET_DEVICES += pine64_pine64-plus

define Device/pine64_sopine-baseboard
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := SoPine
  DEVICE_PACKAGES := kmod-rtl8723bs rtl8723bu-firmware
  $(Device/sun50i-a64)
endef
TARGET_DEVICES += pine64_sopine-baseboard

define Device/xunlong_orangepi-one-plus
  $(Device/sun50i-h6)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi One Plus
  SUNXI_DTS_DIR := allwinner/
endef
TARGET_DEVICES += xunlong_orangepi-one-plus

define Device/xunlong_orangepi-pc2
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi PC 2
  $(Device/sun50i-h5)
endef
TARGET_DEVICES += xunlong_orangepi-pc2

define Device/xunlong_orangepi-zero2
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Zero 2
  $(Device/sun50i-h616)
endef
TARGET_DEVICES += xunlong_orangepi-zero2

define Device/xunlong_orangepi-zero3
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Zero 3
  $(Device/sun50i-h618)
endef
TARGET_DEVICES += xunlong_orangepi-zero3

define Device/xunlong_orangepi-zero-plus
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Zero Plus
  $(Device/sun50i-h5)
endef
TARGET_DEVICES += xunlong_orangepi-zero-plus
