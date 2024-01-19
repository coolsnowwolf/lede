# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023-2024 Ailick <277498654@qq.com>

### Image scripts ###
# This function can be used to change the uboot environment variables.
# In e2000.mk, use this function like this:
# IMAGE/factory.bin := change-uboot-env $(1) $$(UBOOT_PATH) | append-uboot | xxx
define Build/change-uboot-env
	$(if $(findstring demo,$(word 1,$(1))), sed -i 's/spi0.0/spi1.0/g' ./bin/uboot-env.txt)
	$(if $(findstring miniitx,$(word 1,$(1))), sed -i 's/spi1.0/spi0.0/g' ./bin/uboot-env.txt)
	mkenvimage -s 0x1000 -o uboot-env.bin ./bin/uboot-env.txt
	dd if=uboot-env.bin of=$(word 2,$(1)) bs=1k seek=1472 conv=notrunc
	rm -f uboot-env.bin
endef

### Devices ###
define Device/Default
  PROFILES := Default
  KERNEL_NAME := Image
  KERNEL_LOADADDR := 0x80080000
  KERNEL := kernel-bin | gzip | uImage gzip
  DEVICE_DTS_DIR := $(DTS_DIR)/phytium
  DEVICE_DTS = $(1)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-dtb | pad-to 64k | append-kernel \
	  | pad-to 6208k | append-rootfs | pad-rootfs | append-metadata
  DEVICE_PACKAGES := uboot-envtools partx-utils e2fsprogs mkf2fs kmod-gpio-button-hotplug
endef
