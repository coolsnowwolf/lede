# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/linksys_lgs328
  $(Device/uimage-rt-loader)
  SOC := rtl9301
  IMAGE_SIZE := 29696k
  KERNEL_SIZE := 10240k
  DEVICE_VENDOR := Linksys
  BELKIN_MODEL := BKS-RTL93xx
  BELKIN_HEADER := 0x07600001
  PAGESIZE := 2048
  BLOCKSIZE := 128k
  KERNEL := \
	$$(KERNEL) | \
	pad-to $$(BLOCKSIZE)
  IMAGE/sysupgrade.bin := \
	append-rootfs | \
	pad-rootfs | \
	sysupgrade-tar rootfs=$$$$@ | \
	append-metadata
endef

define Device/linksys_lgs328c
  $(Device/linksys_lgs328)
  DEVICE_MODEL := LGS328C
  LINKSYS_MODEL := 60412040
endef
TARGET_DEVICES += linksys_lgs328c

define Device/linksys_lgs328mpc-v2
  $(Device/linksys_lgs328)
  DEVICE_MODEL := LGS328MPC
  DEVICE_VARIANT := v2
  DEVICE_PACKAGES += kmod-hwmon-lm63 kmod-pse-realtek-mcu-i2c
  LINKSYS_MODEL := 60412060
endef
TARGET_DEVICES += linksys_lgs328mpc-v2
