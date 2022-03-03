# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

# On ARC initramfs is put before entry point and so entry point moves
# in memory from build to built. Thus we need to extract EP from vmlinux
# every time before generation of uImage.
kernel_ep = `$(KERNEL_CROSS)readelf -h $(1) | grep "Entry point address" | grep -o 0x.*`

define Build/patch-dtb
	$(STAGING_DIR_HOST)/bin/patch-dtb $@ $(DTS_DIR)/$(DEVICE_DTS).dtb
endef

ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
# Root FS built-in
define Device/vmlinux
	KERNEL_SUFFIX := .elf
	KERNEL := kernel-bin | patch-dtb
	KERNEL_INITRAMFS_NAME = vmlinux-initramfs.elf
endef

define Device/nsim_hs
	$(call Device/vmlinux)
	DEVICE_VENDOR := Synopsys
	DEVICE_MODEL := nSIM HS
	DEVICE_PROFILE := nsim_hs
	DEVICE_DTS := haps_hs
endef
TARGET_DEVICES += nsim_hs
endif

# Root FS on SD-card
KERNEL_LOADADDR := 0x90000000
DEVICE_DTS_LIST:= axs103_idu haps_hs hsdk
FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_AXS10X_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Image/Prepare
	# Build .dtb for all boards we may run on
	$(foreach dts,$(DEVICE_DTS_LIST),
		$(call Image/BuildDTB,$(DTS_DIR)/$(dts).dts,$(DTS_DIR)/$(dts).dtb)
	)
endef

define Image/Build/SDCard
	rm -f $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img
	mkfs.fat $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img -C $(FAT32_BLOCKS)
	mkimage -C none -A arc -T script -d uEnv.txt $(BIN_DIR)/uEnv.scr
	mkenvimage -s 0x4000 -o $(BIN_DIR)/uboot.env ./uboot.env.txt
	mcopy -i $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img $(BIN_DIR)/uEnv.scr ::boot.scr
	mcopy -i $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img $(DTS_DIR)/*.dtb ::
	mcopy -i $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage ::uImage
	mcopy -i $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img $(BIN_DIR)/uboot.env ::uboot.env

	./gen_axs10x_sdcard_img.sh \
		$(BIN_DIR)/$(IMG_PREFIX)-$(PROFILE)-sdcard-vfat-$(1).img \
		$(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-boot.img \
		$(KDIR)/root.$(1) \
		$(CONFIG_AXS10X_SD_BOOT_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)

ifneq ($(CONFIG_TARGET_IMAGES_GZIP),)
	gzip -f9n $(BIN_DIR)/$(IMG_PREFIX)-$(PROFILE)-sdcard-vfat-$(1).img
endif
endef

define Image/BuildKernel
	# Build unified uImage
	$(call Image/BuildKernel/MkuImage, \
		none, $(KERNEL_LOADADDR),$(call kernel_ep,$(KDIR)/vmlinux.elf) , \
		$(KDIR)/vmlinux, \
		$(BIN_DIR)/$(IMG_PREFIX)-uImage \
	)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call Image/Build/SDCard,$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	$(call Image/Gzip,$(BIN_DIR)/$(IMG_PREFIX)-root.$(1))
endef

$(eval $(call BuildImage))
