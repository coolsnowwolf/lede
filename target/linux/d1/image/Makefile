#
# Copyright (C) 2023 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_D1_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

KERNEL_LOADADDR:=0x40200000

define Build/riscv-sdcard
	rm -f $@.boot #$(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img
	mkfs.fat $@.boot  -C $(FAT32_BLOCKS)

	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-boot.scr ::boot.scr
	mcopy -i $@.boot $(DTS_DIR)/$(DEVICE_DTS).dtb ::dtb
	mcopy -i $@.boot $(IMAGE_KERNEL) ::Image

	./gen_d1_sdcard_img.sh \
		$@ \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_D1_SD_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-u-boot-sunxi-with-spl.bin
endef

define Device/Default
  PROFILES := Default
  KERNEL_NAME := Image
  KERNEL := kernel-bin
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := riscv-sdcard | append-metadata | gzip
endef

define Device/FitImageGzip
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/FitImage
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | fit none $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

define Device/dongshan_nezha_stu
  $(call Device/Default)
  DEVICE_VENDOR := Dongshan
  DEVICE_MODEL := Nezha STU devkit
  DEVICE_DTS := allwinner/sun20i-d1-dongshan-nezha-stu
  UBOOT := dongshan_nezha_stu
endef
TARGET_DEVICES += dongshan_nezha_stu

define Device/lichee_rv_dock
  $(call Device/Default)
  DEVICE_VENDOR := Sipeed
  DEVICE_MODEL := LicheePi RV (dock)
  DEVICE_DTS := allwinner/sun20i-d1-lichee-rv-dock
  DEVICE_PACKAGES += kmod-rtl8723bs
  UBOOT := lichee_rv_dock
endef
TARGET_DEVICES += lichee_rv_dock

define Device/mangopi_mq_pro
  $(call Device/Default)
  DEVICE_VENDOR := MangoPi
  DEVICE_MODEL := MQ Pro
  DEVICE_DTS := allwinner/sun20i-d1-mangopi-mq-pro
  DEVICE_PACKAGES += kmod-rtl8723bs
  UBOOT := mangopi_mq_pro
endef
TARGET_DEVICES += mangopi_mq_pro

define Device/nezha
  $(call Device/Default)
  DEVICE_VENDOR := Nezha
  DEVICE_MODEL := D1
  DEVICE_DTS := allwinner/sun20i-d1-nezha
  UBOOT := nezha
endef
TARGET_DEVICES += nezha

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

$(eval $(call BuildImage))
