# 
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_BRCM2708_SD_BOOT_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

### Image scripts ###
define Build/kernel-img
	perl $(LINUX_DIR)/scripts/mkknlimg $@ $@.tmp
	mv $@.tmp $@
endef

define Build/boot-img
	rm -f $@.boot
	mkfs.fat -C $@.boot $(FAT32_BLOCKS)
	mcopy -i $@.boot $(KDIR)/COPYING.linux ::
	mcopy -i $@.boot $(KDIR)/bootcode.bin ::
	mcopy -i $@.boot $(KDIR)/LICENCE.broadcom ::
	mcopy -i $@.boot $(KDIR)/start.elf ::
	mcopy -i $@.boot $(KDIR)/start_cd.elf ::
	mcopy -i $@.boot $(KDIR)/fixup.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_cd.dat ::
	mcopy -i $@.boot cmdline.txt ::
	mcopy -i $@.boot config.txt ::
	mcopy -i $@.boot $(IMAGE_KERNEL) ::$(KERNEL_IMG)
	$(foreach dts,$(shell echo $(DEVICE_DTS)),mcopy -i $@.boot $(DTS_DIR)/$(dts).dtb ::;)
	mmd -i $@.boot ::/overlays
	mcopy -i $@.boot $(DTS_DIR)/overlays/*.dtbo ::/overlays/
	mcopy -i $@.boot $(DTS_DIR)/overlays/README ::/overlays/
endef

define Build/sdcard-img
	./gen_rpi_sdcard_img.sh $@ $@.boot $(IMAGE_ROOTFS) \
		$(CONFIG_BRCM2708_SD_BOOT_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

### Devices ###
define Device/Default
  FILESYSTEMS := ext4
  KERNEL := kernel-bin | kernel-img
  KERNEL_IMG := kernel.img
  IMAGES := sdcard.img.gz
  IMAGE/sdcard.img.gz := boot-img | sdcard-img | gzip | append-metadata
endef

define Device/rpi
  DEVICE_TITLE := Raspberry Pi B/B+/CM/Zero/ZeroW
  DEVICE_DTS := bcm2708-rpi-b bcm2708-rpi-b-plus bcm2708-rpi-cm bcm2708-rpi-0-w
  SUPPORTED_DEVICES := rpi-b rpi-b-plus rpi-cm rpi-zero rpi-zero-w raspberrypi,model-b raspberrypi,model-b-plus raspberrypi,compute-module-1 raspberrypi,model-b-rev2 raspberrypi,model-zero raspberrypi,model-zero-w
endef
ifeq ($(SUBTARGET),bcm2708)
  TARGET_DEVICES += rpi
endif

define Device/rpi-2
  DEVICE_TITLE := Raspberry Pi 2B/3B/3CM
  DEVICE_DTS := bcm2709-rpi-2-b bcm2710-rpi-3-b bcm2710-rpi-cm3
  SUPPORTED_DEVICES := rpi-2-b rpi-3-b rpi-cm raspberrypi,2-model-b raspberrypi,3-model-b raspberrypi,compute-module-3
endef
ifeq ($(SUBTARGET),bcm2709)
  TARGET_DEVICES += rpi-2
endif

define Device/rpi-3
  KERNEL_IMG := kernel8.img
  DEVICE_TITLE := Raspberry Pi 3B (64 bit)
  DEVICE_DTS := broadcom/bcm2710-rpi-3-b
  SUPPORTED_DEVICES := rpi-3-b raspberrypi,3-model-b
endef
ifeq ($(SUBTARGET),bcm2710)
  TARGET_DEVICES += rpi-3
endif

$(eval $(call BuildImage))
