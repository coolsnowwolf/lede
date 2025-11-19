# SPDX-License-Identifier: GPL-2.0-only

define Device/cameo-fw
  CAMEO_BOARD_MODEL = $$(DEVICE_MODEL)
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip | \
	cameo-tag
  IMAGES += factory_image1.bin
  IMAGE/factory_image1.bin := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	pad-to 16 | \
	check-size | \
	cameo-version | \
	cameo-headers
endef

define Device/d-link_dgs-1210
  $(Device/cameo-fw)
  IMAGE_SIZE := 13824k
  DEVICE_VENDOR := D-Link
  CAMEO_KERNEL_PART_SIZE := 1572864
  CAMEO_KERNEL_PART := 2
  CAMEO_ROOTFS_PART := 3
  CAMEO_CUSTOMER_SIGNATURE := 2
  CAMEO_BOARD_VERSION := 32
endef

define Device/hpe_1920
  DEVICE_VENDOR := HPE
  IMAGE_SIZE := 29632k
  BLOCKSIZE := 64k
  H3C_PRODUCT_ID := 0x3c010501
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	7z | \
	h3c-image | \
	h3c-vfs
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	7z | \
	h3c-image
  IMAGE/sysupgrade.bin := \
	append-kernel | \
	pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata
endef

define Device/hwmon-fan-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Fan control switched to hwmon. Your fans will retain \
	bootloader speed unless another control scheme is in place. \
	Config cannot be kept due to conflict in gpio_switch config 'fan_ctrl' under \
	/etc/config/system.
endef

define Device/zyxel_gs1900
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := Dual firmware paritition merged due to size constraints. \
	Upgrade requires a new factory install. Regular sysupgrade is not possible.
  DEVICE_VENDOR := Zyxel
  IMAGE_SIZE := 13952k
  UIMAGE_MAGIC := 0x83800000
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	zyxel-vers | \
	uImage gzip | \
	check-size 6976k
endef
