# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 Bootlin
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/boot-img-ext4
	rm -fR $@.boot
	mkdir -p $@.boot
	$(foreach dts,$(DEVICE_DTS), $(CP) $(KDIR)/image-$(dts).dtb $@.boot/$(dts).dtb;)
	$(CP) $(IMAGE_KERNEL) $@.boot/$(KERNEL_IMG)
	$(INSTALL_DIR) $@.boot/extlinux
	$(CP) ./extlinux.conf $@.boot/extlinux/
	$(SED) 's/@KERNEL@/$(KERNEL_IMG)/' $@.boot/extlinux/extlinux.conf
	$(SED) 's/@DEVICE@/$(DEVICE_NAME)/' $@.boot/extlinux/extlinux.conf
	$(SED) 's/@DTS@/$(DEVICE_DTS)/' $@.boot/extlinux/extlinux.conf
	$(SED) 's/@ROOT@/PARTUUID=$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/05/')/' $@.boot/extlinux/extlinux.conf

	make_ext4fs -J -L kernel -l $(CONFIG_TARGET_KERNEL_PARTSIZE)M \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		$@.bootimg $@.boot
endef

define Build/sdcard-img
	GUID=$(IMG_PART_DISKGUID) ./gen_stm32_sdcard_img.sh \
	     $@ $(STAGING_DIR_IMAGE)/tf-a-$(DEVICE_NAME).stm32 \
	     $(STAGING_DIR_IMAGE)/fip-$(DEVICE_NAME).bin $@.bootimg $(IMAGE_ROOTFS) \
	     $(ENV_SIZE) $(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

define Device/Default
  PROFILES := Default
  DEVICE_VENDOR := STMicroelectronics
  IMAGES := factory.img.gz sysupgrade.img.gz
  IMAGE/factory.img.gz := boot-img-ext4 | sdcard-img | gzip
  IMAGE/sysupgrade.img.gz := boot-img-ext4 | sdcard-img | gzip | append-metadata
  KERNEL := kernel-bin
  KERNEL_NAME := zImage
  KERNEL_IMG := zImage
  DEVICE_DTS_DIR := $(DTS_DIR)/st
  ENV_SIZE := 0x200000
  DEVICE_PACKAGES := kmod-brcmfmac \
		     murata-firmware-43430-sdio \
		     murata-nvram-43430-sdio \
		     wpad-basic-mbedtls \
		     kmod-phy-stm32-usbphyc \
		     kmod-usb2 \
		     kmod-usb-storage \
		     kmod-usb-ledtrig-usbport \
		     -mtd
endef

define Device/stm32mp135f-dk
  DEVICE_MODEL := STM32MP135F-DK
  DEVICE_DTS := stm32mp135f-dk
  SUPPORTED_DEVICES := st,stm32mp135f-dk
endef

define Device/stm32mp157c-dk2
  DEVICE_MODEL := STM32MP157C-DK2
  DEVICE_DTS := stm32mp157c-dk2
  SUPPORTED_DEVICES := st,stm32mp157c-dk2 \
		       st,stm32mp157f-dk2
endef

define Device/stm32mp157c-dk2-scmi
  DEVICE_MODEL := STM32MP157C-DK2
  DEVICE_VARIANT := SCMI
  DEVICE_DTS := stm32mp157c-dk2-scmi
  SUPPORTED_DEVICES := st,stm32mp157c-dk2-scmi \
		       st,stm32mp157f-dk2-scmi
endef

TARGET_DEVICES += stm32mp135f-dk \
		  stm32mp157c-dk2 \
		  stm32mp157c-dk2-scmi

$(eval $(call BuildImage))
