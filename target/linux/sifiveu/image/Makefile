# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 Toco Technologies <info@toco.ae>
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_SIFIVEU_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

KERNEL_LOADADDR:=0x80200000

define Build/riscv-sdcard
	rm -f $@.boot #$(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img
	mkfs.fat $@.boot  -C $(FAT32_BLOCKS)

	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-boot.scr ::boot.scr
	mcopy -i $@.boot $(DTS_DIR)/$(DEVICE_DTS).dtb ::dtb
	mcopy -i $@.boot $(IMAGE_KERNEL) ::Image

	./gen_sifiveu_sdcard_img.sh \
		$@ \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_SIFIVEU_SD_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-u-boot.itb \
		$(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-u-boot.itb-spl
endef

define Device/Default
  PROFILES := Default
  KERNEL_NAME := Image
  KERNEL := kernel-bin | libdeflate-gzip
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := riscv-sdcard | append-metadata | gzip
endef

define Device/sifive_unleashed
  DEVICE_VENDOR := SiFive
  DEVICE_MODEL := Unleashed (FU540)
  DEVICE_DTS := sifive/hifive-unleashed-a00
  UBOOT := sifive_unleashed
endef
TARGET_DEVICES += sifive_unleashed

define Device/sifive_unmatched
  DEVICE_VENDOR := SiFive
  DEVICE_MODEL := Unmatched (FU740)
  DEVICE_DTS := sifive/hifive-unmatched-a00
  DEVICE_PACKAGES += kmod-eeprom-at24 kmod-hwmon-lm90
  UBOOT := sifive_unmatched
endef
TARGET_DEVICES += sifive_unmatched

$(eval $(call BuildImage))
