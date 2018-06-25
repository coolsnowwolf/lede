#
# Copyright (C) 2012-2016 OpenWrt.org
# Copyright (C) 2016 LEDE-project.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

JFFS2_BLOCKSIZE = 128k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR := 0x00008000

SIGNATURE:=$(shell printf "%.8s" $(SOURCE_DATE_EPOCH))

define Build/dtb
	$(call Image/BuildDTB,$(DTS_DIR)/$(DEVICE_DTS).dts,$@.dtb)
endef

# SD-Card Images:
# these values are optimized for a 4GB labeled sdcard that actually holds 7744512 sectors of 512 byte
# MBR:            2048 sectors
# Partition 1:   32768 sectors
# Partition 2:   98304 sectors (configurable)

define Build/boot-scr
	rm -f $@-boot.scr
	sed -e 's#@ROOT@#$(SIGNATURE)#g' \
		$(DEVICE_NAME).bootscript > $@-new.bootscript
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $@-new.bootscript $@-boot.scr
endef

define Build/boot-img
	rm -f $@.boot
	mkfs.fat -C $@.boot 16384
	$(foreach dts,$(DEVICE_DTS), mcopy -i $@.boot $(DTS_DIR)/$(dts).dtb ::$(dts).dtb;)
	mcopy -i $@.boot $(IMAGE_KERNEL) ::$(KERNEL_NAME)
	-mcopy -i $@.boot $@-boot.scr ::boot.scr
endef

define Build/boot-img-ext4
	rm -fR $@.boot
	mkdir -p $@.boot
	$(foreach dts,$(DEVICE_DTS), $(CP) $(DTS_DIR)/$(dts).dtb $@.boot;)
	$(CP) $(IMAGE_KERNEL) $@.boot/$(KERNEL_NAME)
	-$(CP) $@-boot.scr $@.boot/boot.scr
	make_ext4fs -J -l 16384K $@.bootimg $@.boot
endef

define Build/sdcard-img
	if [ -n "$(UBOOT)" ]; then UBOOT="$(STAGING_DIR_IMAGE)/$(UBOOT)"; fi; \
	ROOTFS_SIZE=$$(( $(CONFIG_TARGET_ROOTFS_PARTSIZE) * 1024 * 2 )); \
	SIGNATURE="$(SIGNATURE)" \
	./gen_mvebu_sdcard_img.sh $@ \
		$$UBOOT \
		c 32768 $@.boot \
		83 $$ROOTFS_SIZE $(IMAGE_ROOTFS)
endef

define Build/sdcard-img-ext4
	if [ -n "$(UBOOT)" ]; then UBOOT="$(STAGING_DIR_IMAGE)/$(UBOOT)"; fi; \
	ROOTFS_SIZE=$$(( $(CONFIG_TARGET_ROOTFS_PARTSIZE) * 1024 * 2 )); \
	SIGNATURE="$(SIGNATURE)" \
	./gen_mvebu_sdcard_img.sh $@ \
		$$UBOOT \
		83 32768 $@.bootimg \
		83 $$ROOTFS_SIZE $(IMAGE_ROOTFS)
endef

define Build/omnia-medkit-initramfs
	$(TAR) -c -T /dev/null -f $@
	rm -rf $(dir $(IMAGE_KERNEL))boot
	mkdir -p $(dir $(IMAGE_KERNEL))boot/boot/
	cp $(KDIR)/zImage-initramfs $(dir $(IMAGE_KERNEL))boot/boot/zImage
	cp $(DTS_DIR)/$(DEVICE_DTS).dtb $(dir $(IMAGE_KERNEL))boot/boot/dtb
	$(TAR) -rp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		--file=$@ -C $(dir $(IMAGE_KERNEL))boot/ .
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS := $(1)
  BOARD_NAME = $$(DEVICE_DTS)
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  SUPPORTED_DEVICES = $$(DEVICE_DTS)
  UBOOT :=
endef
DEVICE_VARS += UBOOT

define Device/UBI
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/UBI-factory
  $(Device/UBI)
  UBINIZE_OPTS := -E 5
  IMAGES += factory.img
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | pad-to $$$$(PAGESIZE)
endef

define Device/NAND-128K
  $(Device/UBI)
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  VID_HDR_OFFSET := 2048
endef

define Device/NAND-256K
  $(Device/UBI)
  BLOCKSIZE := 256k
  PAGESIZE := 4096
endef

define Device/NAND-512K
  $(Device/UBI)
  BLOCKSIZE := 512k
  PAGESIZE := 4096
endef

include cortex-a9.mk
include cortex-a53.mk
include cortex-a72.mk

$(eval $(call BuildImage))
