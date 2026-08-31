# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/linksys_lgs352c
  $(Device/uimage-rt-loader)
  SOC := rtl9311
  IMAGE_SIZE := 29696k
  KERNEL_SIZE := 10240k
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := LGS352C
  DEVICE_PACKAGES += kmod-hwmon-lm63
  BELKIN_MODEL := BKS-RTL93xx
  BELKIN_HEADER := 0x07600001
  LINKSYS_MODEL := 60422050
  PAGESIZE := 2048
  BLOCKSIZE := 128k
  UBINIZE_OPTS := -E 5
  KERNEL := \
	$$(KERNEL) | \
	pad-to $$(BLOCKSIZE)
  IMAGE/sysupgrade.bin := \
	append-rootfs | \
	pad-rootfs | \
	sysupgrade-tar rootfs=$$$$@ | \
	append-metadata
endef
TARGET_DEVICES += linksys_lgs352c
