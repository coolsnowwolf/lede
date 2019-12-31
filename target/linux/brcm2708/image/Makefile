# 
# Copyright (C) 2012-2019 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_KERNEL_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

### Image scripts ###
define Build/kernel-img
	perl $(LINUX_DIR)/scripts/mkknlimg $@ $@.tmp
	mv $@.tmp $@
endef

define Build/boot-common
	rm -f $@.boot
	mkfs.fat -C $@.boot $(FAT32_BLOCKS)
	mcopy -i $@.boot $(KDIR)/COPYING.linux ::
	mcopy -i $@.boot $(KDIR)/LICENCE.broadcom ::
	mcopy -i $@.boot cmdline.txt ::
	mcopy -i $@.boot config.txt ::
	mcopy -i $@.boot distroconfig.txt ::
	mcopy -i $@.boot $(IMAGE_KERNEL) ::$(KERNEL_IMG)
	$(foreach dts,$(shell echo $(DEVICE_DTS)),mcopy -i $@.boot $(DTS_DIR)/$(dts).dtb ::;)
	mmd -i $@.boot ::/overlays
	mcopy -i $@.boot $(DTS_DIR)/overlays/*.dtbo ::/overlays/
	mcopy -i $@.boot $(DTS_DIR)/overlays/README ::/overlays/
endef

define Build/boot-2708
	mcopy -i $@.boot $(KDIR)/bootcode.bin ::
	mcopy -i $@.boot $(KDIR)/start.elf ::
	mcopy -i $@.boot $(KDIR)/start_cd.elf ::
	mcopy -i $@.boot $(KDIR)/start_x.elf ::
	mcopy -i $@.boot $(KDIR)/fixup.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_cd.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_x.dat ::
endef

define Build/boot-2711
	mcopy -i $@.boot $(KDIR)/start4.elf ::
	mcopy -i $@.boot $(KDIR)/start4cd.elf ::
	mcopy -i $@.boot $(KDIR)/start4x.elf ::
	mcopy -i $@.boot $(KDIR)/fixup4.dat ::
	mcopy -i $@.boot $(KDIR)/fixup4cd.dat ::
	mcopy -i $@.boot $(KDIR)/fixup4x.dat ::
endef

define Build/sdcard-img
	./gen_rpi_sdcard_img.sh $@ $@.boot $(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

### Devices ###
define Device/Default
  DEVICE_VENDOR := Raspberry Pi
  KERNEL := kernel-bin | kernel-img
  KERNEL_IMG := kernel.img
  IMAGES := factory.img.gz sysupgrade.img.gz
  IMAGE/sysupgrade.img.gz := boot-common | boot-2708 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2708 | sdcard-img | gzip
endef

define Device/rpi
  DEVICE_MODEL := B/B+/CM/Zero/ZeroW
  DEVICE_DTS := \
	bcm2708-rpi-b bcm2708-rpi-b-plus \
	bcm2708-rpi-cm \
	bcm2708-rpi-zero bcm2708-rpi-zero-w
  SUPPORTED_DEVICES := \
	rpi-b rpi-b-plus rpi-cm rpi-zero rpi-zero-w \
	raspberrypi,model-b raspberrypi,model-b-plus raspberrypi,model-b-rev2 \
	raspberrypi,compute-module raspberrypi,compute-module-1 \
	raspberrypi,model-zero raspberrypi,model-zero-w
  DEVICE_PACKAGES := \
	cypress-firmware-43430-sdio \
	cypress-nvram-43430-sdio-rpi-zero-w \
	kmod-brcmfmac wpad-basic
endef
ifeq ($(SUBTARGET),bcm2708)
  TARGET_DEVICES += rpi
endif

define Device/rpi-2
  DEVICE_MODEL := 2B/3B/3B+/3CM/4B
  DEVICE_DTS := \
	bcm2709-rpi-2-b bcm2710-rpi-2-b \
	bcm2710-rpi-3-b bcm2710-rpi-3-b-plus \
	bcm2711-rpi-4-b \
	bcm2710-rpi-cm3
  SUPPORTED_DEVICES := \
	rpi-2-b rpi-3-b rpi-3-b-plus rpi-cm \
	raspberrypi,2-model-b raspberrypi,2-model-b-rev2 \
	raspberrypi,3-model-b raspberrypi,3-model-b-plus \
	raspberrypi,3-compute-module raspberrypi,compute-module-3 \
	raspberrypi,4-model-b
  DEVICE_PACKAGES := \
	cypress-firmware-43430-sdio \
	cypress-nvram-43430-sdio-rpi-3b \
	cypress-firmware-43455-sdio \
	cypress-nvram-43455-sdio-rpi-3b-plus cypress-nvram-43455-sdio-rpi-4b \
	kmod-brcmfmac wpad-basic
  IMAGE/sysupgrade.img.gz := boot-common | boot-2708 | boot-2711 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2708 | boot-2711 | sdcard-img | gzip
endef
ifeq ($(SUBTARGET),bcm2709)
  TARGET_DEVICES += rpi-2
endif

define Device/rpi-3
  DEVICE_MODEL := 2B-1.2/3B/3B+/3CM
  KERNEL_IMG := kernel8.img
  DEVICE_DTS := \
	broadcom/bcm2710-rpi-2-b \
	broadcom/bcm2710-rpi-3-b broadcom/bcm2710-rpi-3-b-plus \
	broadcom/bcm2710-rpi-cm3
  SUPPORTED_DEVICES := \
	rpi-3-b rpi-3-b-plus \
	raspberrypi,2-model-b-rev2 \
	raspberrypi,3-model-b raspberrypi,3-model-b-plus \
	raspberrypi,3-compute-module raspberrypi,compute-module-3
  DEVICE_PACKAGES := \
	cypress-firmware-43430-sdio \
	cypress-nvram-43430-sdio-rpi-3b \
	cypress-firmware-43455-sdio \
	cypress-nvram-43455-sdio-rpi-3b-plus \
	kmod-brcmfmac wpad-basic
endef
ifeq ($(SUBTARGET),bcm2710)
  TARGET_DEVICES += rpi-3
endif

define Device/rpi-4
  DEVICE_MODEL := 4B
  KERNEL_IMG := kernel8.img
  DEVICE_DTS := broadcom/bcm2711-rpi-4-b
  SUPPORTED_DEVICES := \
	raspberrypi,4-model-b
  DEVICE_PACKAGES := \
	cypress-firmware-43455-sdio \
	cypress-nvram-43455-sdio-rpi-4b \
	kmod-brcmfmac wpad-basic
  IMAGE/sysupgrade.img.gz := boot-common | boot-2711 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2711 | sdcard-img | gzip
endef
ifeq ($(SUBTARGET),bcm2711)
  TARGET_DEVICES += rpi-4
endif

$(eval $(call BuildImage))
