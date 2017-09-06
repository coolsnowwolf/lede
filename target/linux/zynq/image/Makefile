#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

#################################################
# Images
#################################################

# $(1): rootfs contents directory
define Image/mkfs/cpio
	( cd $(call mkfs_target_dir,$(1))/; find . | cpio -o -H newc | gzip -9n > $@ )
endef

# $(1): zynq board id
define Build/dtb
	$(call Image/BuildDTB,$(DTS_DIR)/zynq-$(1).dts,$@.dtb)
endef

define Build/uRamdisk
	# Create uboot cpio.gz
	mkimage -A arm -T ramdisk -C gzip -n "$(PROFILE) OpenWRT rootfs" \
		-d $(IMAGE_ROOTFS) $@.new
	mv $@.new $@
endef

# $(1): FIT name/description
define Build/fit
	# create FIT image with rootfs
	./mkits.sh \
		-D $(1) -o $@.its -k $(IMAGE_KERNEL) -d $(IMAGE_KERNEL).dtb \
		-C none -a $(KERNEL_LOADADDR) -e $(KERNEL_ENTRY) \
		-A $(ARCH) -v $(LINUX_VERSION) -r $(IMAGE_ROOTFS) -z gzip

	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@
endef

#################################################
# Devices
#################################################

# default kernel load address
KERNEL_LOADADDR=0x8000
KERNEL_ENTRY=0x8000

TARGET_FILESYSTEMS += cpio

# $(1): canonical device name
# $(2): lowercase device id
define ZynqDevice
  define Device/$(1)
	DEVICE_TITLE := $(1) development board
	KERNEL := dtb $(2) | kernel-bin | uImage none
	KERNEL_NAME = zImage
	IMAGES := uramdisk.image.gz fit.itb
	IMAGE/uramdisk.image.gz := uRamdisk
	IMAGE/fit.itb := fit $(2)
	IMAGE_NAME = $$$$(IMG_PREFIX)-$(2)-$$$$(2)
  endef
  TARGET_DEVICES += $(1)
endef

$(eval $(call ZynqDevice,ZC702,zc702))
$(eval $(call ZynqDevice,ZedBoard,zed))
$(eval $(call ZynqDevice,ZYBO,zybo))

$(eval $(call BuildImage))
