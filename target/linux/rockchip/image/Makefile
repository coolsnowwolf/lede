# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DEVICE_VARS += UBOOT_DEVICE_NAME
DEVICE_VARS += IMAGE_PART_OFFSET IMAGE_PART_ALIGN IMAGE_GPT IMAGE_BOOTFS
DEVICE_VARS += BOOTLOADER_IMAGE BOOTLOADER_OFFSET ENV_IMAGE ENV_OFFSET

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

### Image scripts ###
define Build/boot-common
	# This creates a new folder copies the dtb (as rockchip.dtb) 
	# and the kernel image (as kernel.img)
	rm -fR $@.boot
	mkdir -p $@.boot

	$(CP) $(KDIR)/image-$(firstword $(notdir $(DEVICE_DTS))).dtb $@.boot/rockchip.dtb
	$(CP) $(IMAGE_KERNEL) $@.boot/kernel.img
endef

define Build/boot-img-ext4
	rm -fR $@.boot
	mkdir -p $@.boot
	$(CP) $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $@.boot/rockchip.dtb
	$(CP) $(IMAGE_KERNEL) $@.boot/kernel.img
	$(INSTALL_DIR) $@.boot/extlinux
	$(CP) ./extlinux.conf $@.boot/extlinux/
	$(SED) 's/@ROOT@/PARTUUID=$(shell echo $(IMG_PART_DISKGUID) | sed 's/00$$/05/')/' $@.boot/extlinux/extlinux.conf
	make_ext4fs -J -L BOOT -l $(CONFIG_TARGET_KERNEL_PARTSIZE)M \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		$@.bootimg $@.boot
endef

define Build/boot-script
	# Make an U-boot image and copy it to the boot partition
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $(if $(1),$(1),mmc).bootscript $@.boot/boot.scr
endef

define Build/pine64-img
	# Creates the final SD/eMMC images, 
	# combining boot partition, root partition as well as the u-boot bootloader

	# Generate a new partition table in $@ with 32 MiB of 
	# alignment padding for the idbloader and u-boot to fit:
	# http://opensource.rock-chips.com/wiki_Boot_option#Boot_flow
	#
	# U-Boot SPL expects the U-Boot ITB to be located at sector 0x4000 (8 MiB) on the MMC storage
	$(if $(IMAGE_GPT),GUID="$(IMG_PART_DISKGUID)" PURE_GPT=1) \
	$(if $(IMAGE_PART_OFFSET),KERNEL_OFFSET="$(IMAGE_PART_OFFSET)") \
	$(if $(IMAGE_BOOTFS),KERNEL_IMAGE="$@.bootimg") \
	PADDING=1 $(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		$(if $(IMAGE_PART_ALIGN),$(IMAGE_PART_ALIGN),32768)

	# Copy the idbloader and the u-boot image to the image at sector 0x40 and 0x4000
	$(if $(BOOTLOADER_IMAGE), \
		dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-$(BOOTLOADER_IMAGE) of="$@" bs=512 seek=$(BOOTLOADER_OFFSET) conv=notrunc, \
		dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-idbloader.img of="$@" seek=64 conv=notrunc; \
		dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-u-boot.itb of="$@" seek=16384 conv=notrunc)
	$(if $(ENV_IMAGE),dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-$(ENV_IMAGE) of="$@" bs=512 seek=$(ENV_OFFSET) conv=notrunc)
endef

define Build/pine64-bin
	# Typical Rockchip boot flow with Rockchip miniloader
	# Rockchp idbLoader which is combinded by Rockchip ddr init bin
	# and miniloader bin from Rockchip rkbin project

	# Generate a new partition table in $@ with 32 MiB of alignment
	# padding for the idbloader, uboot and trust image to fit:
	# http://opensource.rock-chips.com/wiki_Boot_option#Boot_flow
	PADDING=1 $(SCRIPT_DIR)/gen_image_generic.sh \
		$@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
		32768

	# Copy the idbloader, uboot and trust image to the image at sector 0x40, 0x4000 and 0x6000
	dd if="$(STAGING_DIR_IMAGE)"/$(SOC)-idbloader.bin of="$@" seek=64 conv=notrunc
	dd if="$(STAGING_DIR_IMAGE)"/$(UBOOT_DEVICE_NAME)-uboot.img of="$@" seek=16384 conv=notrunc
	dd if="$(STAGING_DIR_IMAGE)"/$(SOC)-trust.bin of="$@" seek=24576 conv=notrunc
endef

### Devices ###
define Device/Default
  PROFILES := Default
  KERNEL := kernel-bin
  IMAGES := sysupgrade.img.gz
  DEVICE_DTS_DIR = $$(DTS_DIR)/rockchip
  DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
