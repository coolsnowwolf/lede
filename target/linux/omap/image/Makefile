#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_OMAP_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

UBIFS_OPTS = -F -m 2048 -e 124KiB -c 4096 -U
UBI_OPTS = -m 2048 -p 128KiB -s 512 -O 2048

define Build/omap-sdcard
	rm -f $@.boot
	mkfs.fat $@.boot -C $(FAT32_BLOCKS)

	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)/MLO ::MLO
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)/u-boot.img ::u-boot.img
	mcopy -i $@.boot $(STAGING_DIR_IMAGE)/$(DEVICE_NAME)/boot.scr ::boot.scr
	mmd -i $@.boot ::/dtbs
	mcopy -i $@.boot $(DTS_DIR)/$(DEVICE_DTS).dtb ::/dtbs/$(DEVICE_DTS).dtb
	mcopy -i $@.boot $(IMAGE_KERNEL) ::/zImage
	./gen_omap_sdcard_img.sh $@ \
		$@.boot \
		$(IMAGE_ROOTFS) \
		$(CONFIG_OMAP_SD_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	rm -f $@.boot
endef

define Device/Default
  PROFILES := Default
  KERNEL_NAME := zImage
  KERNEL := kernel-bin
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := omap-sdcard | append-metadata | gzip
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
endef

#uboot-omap-am335x_evm uboot-omap-omap3_beagle uboot-omap-omap3_overo uboot-omap-omap4_panda

define Device/ti_am335x-evm
  DEVICE_VENDOR := Texas Instruments
  DEVICE_MODEL := AM335x EVM
  DEVICE_DTS := am335x-evm
endef

TARGET_DEVICES += ti_am335x-evm

define Device/ti_am335x-bone-black
  DEVICE_VENDOR := Texas Instruments
  DEVICE_MODEL := AM335x BeagleBone Black
  DEVICE_DTS := am335x-boneblack
endef

TARGET_DEVICES += ti_am335x-bone-black

define Device/ti_omap4-panda
  DEVICE_VENDOR := PandaBoard.org
  DEVICE_MODEL := OMAP4 TI pandaboard
  DEVICE_DTS := omap4-panda
  DEVICE_PACKAGES := kmod-usb-net-smsc95xx
endef

TARGET_DEVICES += ti_omap4-panda

define Device/ti_omap3-beagle
  DEVICE_VENDOR := BeagleBoard.org
  DEVICE_MODEL := OMAP3 TI beagleboard
  DEVICE_DTS := omap3-beagle
  # beagleboard doesn't have a network interface, support most common usb net
  DEVICE_PACKAGES := kmod-usb-net \
	kmod-usb-net-asix kmod-usb-net-asix-ax88179 kmod-usb-net-hso \
	kmod-usb-net-kaweth kmod-usb-net-pegasus kmod-usb-net-mcs7830 \
	kmod-usb-net-smsc95xx kmod-usb-net-dm9601-ether
endef

TARGET_DEVICES += ti_omap3-beagle

$(eval $(call BuildImage))
