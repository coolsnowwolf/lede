#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_KERNEL_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/zynq-sdcard
	rm -f $@.boot
	mkfs.fat $@.boot -C $(FAT32_BLOCKS)
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-boot.bin ::boot.bin
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-u-boot.img ::u-boot.img
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)-uEnv.txt ::uEnv.txt
	mcopy -i $@.boot $(IMAGE_KERNEL) ::fit.itb
	./gen_zynq_sdcard_img.sh $@ \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	rm -f $@.boot
endef

#################################################
# Default and templates
#################################################

define Device/Default
	PROFILES := Default
	DEVICE_DTS := $(lastword $(subst _, ,$(1)))
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_INITRAMFS_PREFIX := $$(IMG_PREFIX)-$(1)-initramfs
	KERNEL_PREFIX := $$(IMAGE_PREFIX)
	KERNEL_LOADADDR := 0x8000
	IMAGES := sdcard.img.gz
	IMAGE/sdcard.img.gz := zynq-sdcard | gzip
endef

define Device/FitImageGzip
	KERNEL_SUFFIX := -fit-uImage.itb
	KERNEL = kernel-bin | gzip | fit gzip $$(DTS_DIR)/$$(DEVICE_DTS).dtb
	KERNEL_NAME := Image
endef

#################################################
# Devices
#################################################

define Device/avnet_zynq-zed
	$(call Device/FitImageGzip)
	DEVICE_VENDOR := Avnet
	DEVICE_MODEL := ZedBoard
endef
TARGET_DEVICES += avnet_zynq-zed

define Device/digilent_zynq-zybo
	$(call Device/FitImageGzip)
	DEVICE_VENDOR := Digilent
	DEVICE_MODEL := Zybo
endef
TARGET_DEVICES += digilent_zynq-zybo

define Device/digilent_zynq-zybo-z7
	$(call Device/FitImageGzip)
	DEVICE_VENDOR := Digilent
	DEVICE_MODEL := Zybo Z7
endef
TARGET_DEVICES += digilent_zynq-zybo-z7

define Device/xlnx_zynq-zc702
	$(call Device/FitImageGzip)
	DEVICE_VENDOR := Xilinx
	DEVICE_MODEL := ZC702
	DEVICE_PACKAGES:=kmod-can kmod-can-xilinx-can
endef
TARGET_DEVICES += xlnx_zynq-zc702

$(eval $(call BuildImage))
