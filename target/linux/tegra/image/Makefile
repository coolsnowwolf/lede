# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017-2019 Tomasz Maciej Nowak <tmn505@gmail.com>

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/tegra-sdcard
	rm -fR $@.boot
	mkdir -p $@.boot
	$(CP) $(KDIR)/$(KERNEL_NAME) $@.boot
	$(if $(DEVICE_DTS),\
		$(foreach dtb,$(DEVICE_DTS),$(CP) $(DTS_DIR)/$(dtb).dtb $@.boot), \
		$(CP) $(DTS_DIR)/*.dtb $@.boot)
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
		-n '$(DEVICE_TITLE) OpenWrt bootscript' \
		-d $(BOOT_SCRIPT) \
		$@.boot/boot.scr

	SIGNATURE="$(IMG_PART_SIGNATURE)" \
	$(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		2048

	$(if $(UBOOT),dd if=$(STAGING_DIR_IMAGE)/$(UBOOT).img of=$@ bs=512 skip=1 seek=1 conv=notrunc)
endef

DEVICE_VARS += BOOT_SCRIPT UBOOT

define Device/Default
  BOOT_SCRIPT := generic-bootscript
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := tegra-sdcard | gzip | append-metadata
  KERNEL_NAME := zImage
  KERNEL := kernel-bin
  PROFILES := Default
endef

define Device/compulab_trimslice
  DEVICE_VENDOR := CompuLab
  DEVICE_MODEL := TrimSlice
  DEVICE_DTS := tegra20-trimslice
  DEVICE_PACKAGES := kmod-r8169 kmod-rt2800-usb kmod-rtc-em3027 \
	kmod-usb-storage wpad-mini
  UBOOT := trimslice-mmc
endef
TARGET_DEVICES += compulab_trimslice

$(eval $(call BuildImage))
