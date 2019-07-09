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

define Build/boot-scr
	rm -f $@-boot.scr
	sed \
		-e 's#@ROOT@#$(SIGNATURE)#g' \
		-e 's#@DTB@#$(firstword $(DEVICE_DTS))#g' \
		$(BOOT_SCRIPT).bootscript > $@-new.bootscript
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $@-new.bootscript $@-boot.scr
endef

define Build/boot-img
	rm -f $@.boot
	mkfs.fat -C $@.boot $$(( $(CONFIG_TARGET_KERNEL_PARTSIZE) * 1024 ))
	$(foreach dts,$(DEVICE_DTS), mcopy -i $@.boot $(KDIR)/image-$(dts).dtb ::$(dts).dtb;)
	mcopy -i $@.boot $(IMAGE_KERNEL) ::$(KERNEL_NAME)
	-mcopy -i $@.boot $@-boot.scr ::boot.scr
endef

define Build/boot-img-ext4
	rm -fR $@.boot
	mkdir -p $@.boot
	$(foreach dts,$(DEVICE_DTS), $(CP) $(KDIR)/image-$(dts).dtb $@.boot/$(dts).dtb;)
	$(CP) $(IMAGE_KERNEL) $@.boot/$(KERNEL_NAME)
	-$(CP) $@-boot.scr $@.boot/boot.scr
	make_ext4fs -J -l $(CONFIG_TARGET_KERNEL_PARTSIZE)M $@.bootimg $@.boot
endef

define Build/sdcard-img
	SIGNATURE="$(SIGNATURE)" \
	./gen_mvebu_sdcard_img.sh $@ \
		$(if $(UBOOT),$(STAGING_DIR_IMAGE)/$(UBOOT)) \
		c $(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		83 $(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS)
endef

define Build/sdcard-img-ext4
	SIGNATURE="$(SIGNATURE)" \
	./gen_mvebu_sdcard_img.sh $@ \
		$(if $(UBOOT),$(STAGING_DIR_IMAGE)/$(UBOOT)) \
		83 $(CONFIG_TARGET_KERNEL_PARTSIZE) $@.bootimg \
		83 $(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS)
endef

define Build/omnia-medkit-initramfs
	$(TAR) -c -T /dev/null -f $@
	rm -rf $(dir $(IMAGE_KERNEL))boot
	mkdir -p $(dir $(IMAGE_KERNEL))boot/boot/
	cp $(KDIR)/zImage-initramfs $(dir $(IMAGE_KERNEL))boot/boot/zImage
	cp $(KDIR)/image-$(DEVICE_DTS).dtb $(dir $(IMAGE_KERNEL))boot/boot/dtb
	$(TAR) -rp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		--file=$@ -C $(dir $(IMAGE_KERNEL))boot/ .
endef

define Device/Default
  PROFILES := Default
  BOARD_NAME = $$(DEVICE_DTS)
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  IMAGES := sysupgrade.bin
  IMAGE/factory.img := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | pad-to $$$$(PAGESIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
  UBINIZE_OPTS := -E 5
  UBOOT :=
  BOOT_SCRIPT :=
endef
DEVICE_VARS += BOOT_SCRIPT UBOOT

define Device/Default-arm64
  BOOT_SCRIPT := generic-arm64
  DTS_DIR := $(DTS_DIR)/marvell
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-scr | boot-img-ext4 | sdcard-img-ext4 | gzip | append-metadata
  KERNEL_NAME := Image
  KERNEL := kernel-bin
endef

define Device/NAND-128K
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  VID_HDR_OFFSET := 2048
endef

define Device/NAND-256K
  BLOCKSIZE := 256k
  PAGESIZE := 4096
endef

define Device/NAND-512K
  BLOCKSIZE := 512k
  PAGESIZE := 4096
endef

include cortex-a9.mk
include cortex-a53.mk
include cortex-a72.mk

$(eval $(call BuildImage))
