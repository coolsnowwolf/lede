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

DEVICE_VARS += BOOT_SCRIPT UBOOT
KERNEL_LOADADDR := 0x00008000

define Build/boot-scr
	rm -f $@-boot.scr
	sed \
		-e 's#@ROOT@#$(IMG_PART_SIGNATURE)#g' \
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
	make_ext4fs -J -L kernel -l $(CONFIG_TARGET_KERNEL_PARTSIZE)M $@.bootimg $@.boot
endef

define Build/buffalo-kernel-jffs2
	rm -rf $(KDIR)/kernel_jffs2 $@.fakerd
	mkdir -p $(KDIR)/kernel_jffs2
	dd if=/dev/zero of=$@.fakerd bs=131008 count=1 conv=sync
	$(STAGING_DIR_HOST)/bin/mkimage \
		-T ramdisk -A $(LINUX_KARCH) -O linux -C gzip -n 'fake initrd' \
		-d $@.fakerd $(KDIR)/kernel_jffs2/initrd.buffalo
	cp $@ $(KDIR)/kernel_jffs2/uImage.buffalo
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		--little-endian -v --squash-uids -q -f -n -x lzma -x rtime -m none \
		--eraseblock=128KiB --pad=$(KERNEL_SIZE) -d $(KDIR)/kernel_jffs2 -o $@
	rm -rf $(KDIR)/kernel_jffs2 $@.fakerd
endef

define Build/sdcard-img
	SIGNATURE="$(IMG_PART_SIGNATURE)" \
	./gen_mvebu_sdcard_img.sh $@ \
		$(if $(UBOOT),$(STAGING_DIR_IMAGE)/$(UBOOT)) \
		c $(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
		83 $(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS)
endef

define Build/sdcard-img-ext4
	SIGNATURE="$(IMG_PART_SIGNATURE)" \
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

define Build/uDPU-firmware
	(rm -fR $@-fw; mkdir -p $@-fw)
	$(CP) $(BIN_DIR)/$(IMAGE_PREFIX)-initramfs.itb $@-fw/recovery.itb
	$(CP) $@-boot.scr $@-fw/boot.scr
	$(TAR) -cvzp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-f $@-fw/rootfs.tgz -C $(TARGET_DIR) .
	$(TAR) -cvzp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-f $@-fw/boot.tgz -C $@.boot .
	$(TAR) -cvzp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-f $(KDIR_TMP)/$(IMAGE_PREFIX)-firmware.tgz -C $@-fw .
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS = $$(SOC)-$(lastword $(subst _, ,$(1)))
  DEVICE_DTS_DIR := $(DTS_DIR)
  BOARD_NAME = $$(DEVICE_DTS)
  KERNEL_NAME := zImage
  KERNEL := kernel-bin | append-dtb | uImage none
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
  UBINIZE_OPTS := -E 5
  UBOOT :=
  BOOT_SCRIPT :=
endef

define Device/Default-arm64
  BOOT_SCRIPT := generic-arm64
  DEVICE_DTS_DIR := $(DTS_DIR)/marvell
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

include $(SUBTARGET).mk

$(eval $(call BuildImage))
