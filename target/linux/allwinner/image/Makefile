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

define Device/100ask_dongshan-nezha-stu
  $(call Device/Default)
  DEVICE_VENDOR := Dongshan
  DEVICE_MODEL := Nezha STU devkit
  DEVICE_DTS := allwinner/sun20i-d1-dongshan-nezha-stu
  SUPPORTED_DEVICES += dongshan_nezha_stu
  UBOOT := dongshan_nezha_stu
endef
TARGET_DEVICES += 100ask_dongshan-nezha-stu

define Device/allwinner_d1-nezha
  $(call Device/Default)
  DEVICE_VENDOR := Allwinner
  DEVICE_MODEL := D1 Nezha
  DEVICE_DTS := allwinner/sun20i-d1-nezha
  SUPPORTED_DEVICES += nezha
  DEVICE_PACKAGES += kmod-gpio-pcf857x
  UBOOT := nezha
endef
TARGET_DEVICES += allwinner_d1-nezha

define Device/sipeed_lichee-rv-dock
  $(call Device/Default)
  DEVICE_VENDOR := Sipeed
  DEVICE_MODEL := LicheePi RV (dock)
  DEVICE_DTS := allwinner/sun20i-d1-lichee-rv-dock
  SUPPORTED_DEVICES += lichee_rv_dock
  DEVICE_PACKAGES += kmod-rtw88-8723ds wpad-basic-mbedtls
  UBOOT := lichee_rv_dock
endef
TARGET_DEVICES += sipeed_lichee-rv-dock

define Device/widora_mangopi-mq-pro
  $(call Device/Default)
  DEVICE_VENDOR := MangoPi
  DEVICE_MODEL := MQ Pro
  DEVICE_DTS := allwinner/sun20i-d1-mangopi-mq-pro
  SUPPORTED_DEVICES += mangopi_mq_pro
  DEVICE_PACKAGES += kmod-rtw88-8723ds wpad-basic-mbedtls
  UBOOT := mangopi_mq_pro
endef
TARGET_DEVICES += widora_mangopi-mq-pro

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

$(eval $(call BuildImage))
