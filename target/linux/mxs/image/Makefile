# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2015 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

BOARDS:= \
	imx23-olinuxino \
	imx28-duckbill

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_BOOTFS_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Image/BuildKernel
	mkimage -A arm -O linux -T kernel -C none \
		-a 0x40008000 -e 0x40008000 \
		-n 'ARM OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(KDIR)/zImage $(KDIR)/uImage
	cp $(KDIR)/uImage $(BIN_DIR)/$(IMG_PREFIX)-uImage
endef

define Image/InstallKernel
	mkdir -p $(TARGET_DIR)/boot
	cp \
		$(KDIR)/zImage $(KDIR)/uImage \
		$(foreach board,$(BOARDS),$(DTS_DIR)/$(board).dtb) \
		$(TARGET_DIR)/boot/
endef

define Image/Build/SDCard-vfat-ext4
	rm -f $(KDIR)/boot.img
	mkfs.fat $(KDIR)/boot.img -C $(FAT32_BLOCKS)

	mcopy -i $(KDIR)/boot.img $(DTS_DIR)/$(3).dtb ::$(3).dtb
	mcopy -i $(KDIR)/boot.img $(BIN_DIR)/$(IMG_PREFIX)-uImage ::uImage

	./gen_sdcard_vfat_ext4.sh \
		$(BIN_DIR)/$(2) \
		$(STAGING_DIR_IMAGE)/$(4)-u-boot.sb \
		$(KDIR)/boot.img \
		$(KDIR)/root.$(1) \
		$(CONFIG_TARGET_BOOTFS_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	$(call Image/Gzip,$(BIN_DIR)/$(2))
endef

define Image/Build/SDCard-ext4-ext4
	./gen_sdcard_ext4_ext4.sh \
		$(BIN_DIR)/$(2) \
		$(STAGING_DIR_IMAGE)/$(4)-u-boot.sb \
		$(KDIR)/root.$(1) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)
	$(call Image/Gzip,$(BIN_DIR)/$(2))
endef

define Image/Build/Profile/olinuxino-maxi
	$(call Image/Build/SDCard-vfat-ext4,$(1),$(2),imx23-olinuxino,mx23_olinuxino)
endef

define Image/Build/Profile/olinuxino-micro
	$(call Image/Build/SDCard-vfat-ext4,$(1),$(2),imx23-olinuxino,mx23_olinuxino)
endef

define Image/Build/Profile/duckbill
	$(call Image/Build/SDCard-ext4-ext4,$(1),$(2),imx28-duckbill,duckbill)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call Image/Build/Profile/$(PROFILE),$(1),$(IMG_PREFIX)-$(PROFILE)-sdcard.img)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	$(call Image/Gzip,$(BIN_DIR)/$(IMG_PREFIX)-root.$(1))
endef

$(eval $(call BuildImage))
