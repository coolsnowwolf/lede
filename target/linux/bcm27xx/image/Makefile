# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2012-2019 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_TARGET_KERNEL_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))

define Build/Compile
	$(CP) $(LINUX_DIR)/COPYING $(KDIR)/COPYING.linux
endef

### Image scripts ###
define Build/boot-common
	echo $(IMG_PART_SIGNATURE) > $@-partuuid.txt
	sed \
		-e 's#@ROOT@#PARTUUID=$(IMG_PART_SIGNATURE)-02#g' \
		cmdline.txt > $@-cmdline.txt

	rm -f $@.boot
	mkfs.fat -n boot -C $@.boot $(FAT32_BLOCKS)
	mcopy -i $@.boot $(KDIR)/COPYING.linux ::
	$(if $(CONFIG_PACKAGE_bcm27xx-gpu-fw),mcopy -i $@.boot $(KDIR)/LICENCE.broadcom ::,)
	mcopy -i $@.boot $@-cmdline.txt ::cmdline.txt
	mcopy -i $@.boot config.txt ::
	mcopy -i $@.boot distroconfig.txt ::
	mcopy -i $@.boot $@-partuuid.txt ::partuuid.txt
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
	mcopy -i $@.boot $(KDIR)/start_db.elf ::
	mcopy -i $@.boot $(KDIR)/start_x.elf ::
	mcopy -i $@.boot $(KDIR)/fixup.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_cd.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_db.dat ::
	mcopy -i $@.boot $(KDIR)/fixup_x.dat ::
endef

define Build/boot-2711
	mcopy -i $@.boot $(KDIR)/start4.elf ::
	mcopy -i $@.boot $(KDIR)/start4cd.elf ::
	mcopy -i $@.boot $(KDIR)/start4db.elf ::
	mcopy -i $@.boot $(KDIR)/start4x.elf ::
	mcopy -i $@.boot $(KDIR)/fixup4.dat ::
	mcopy -i $@.boot $(KDIR)/fixup4cd.dat ::
	mcopy -i $@.boot $(KDIR)/fixup4db.dat ::
	mcopy -i $@.boot $(KDIR)/fixup4x.dat ::
endef

define Build/sdcard-img
	SIGNATURE="$(IMG_PART_SIGNATURE)" \
	./gen_rpi_sdcard_img.sh $@ $@.boot $(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
endef

### Shared device package groups ###
WIFI_43430_FW := cypress-firmware-43430-sdio brcmfmac-nvram-43430-sdio
WIFI_43455_FW := cypress-firmware-43455-sdio brcmfmac-nvram-43455-sdio
BRCM_WIFI_DRIVER := kmod-brcmfmac wpad-basic-mbedtls
LAN_DRIVERS := kmod-usb-net-lan78xx kmod-usb-net-rtl8152 kmod-r8169

### Devices ###
define Device/Default
  DEVICE_VENDOR := Raspberry Pi
  KERNEL := kernel-bin
  KERNEL_IMG := kernel.img
  IMAGES := factory.img.gz sysupgrade.img.gz
  IMAGE/sysupgrade.img.gz := boot-common | boot-2708 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2708 | sdcard-img | gzip
endef

define Device/rpi
  DEVICE_MODEL := A/A+/B/B+/CM/Zero/ZeroW
  DEVICE_DTS := \
	broadcom/bcm2708-rpi-b broadcom/bcm2708-rpi-b-rev1 \
	broadcom/bcm2708-rpi-b-plus broadcom/bcm2708-rpi-cm \
	broadcom/bcm2708-rpi-zero broadcom/bcm2708-rpi-zero-w
  SUPPORTED_DEVICES := \
	rpi-a rpi-a-plus \
	rpi-b rpi-b-plus \
	rpi-cm \
	rpi-zero rpi-zero-w \
	raspberrypi,model-a raspberrypi,model-a-plus \
	raspberrypi,model-b raspberrypi,model-b-plus raspberrypi,model-b-rev2 \
	raspberrypi,compute-module raspberrypi,compute-module-1 \
	raspberrypi,model-zero raspberrypi,model-zero-w
  DEVICE_PACKAGES := \
	$(WIFI_43430_FW) \
	$(BRCM_WIFI_DRIVER)
endef
ifeq ($(SUBTARGET),bcm2708)
  TARGET_DEVICES += rpi
endif

define Device/rpi-2
  DEVICE_MODEL := 2B/2B 1.2
  DEVICE_VARIANT := (32bit)
  DEVICE_ALT0_VENDOR := Raspberry Pi
  DEVICE_ALT0_MODEL := 3A+/3B/3B+/CM3/Zero2/Zero2W/CM0
  DEVICE_ALT0_VARIANT := (32bit)
  DEVICE_ALT1_VENDOR := Raspberry Pi
  DEVICE_ALT1_MODEL := 4B/400/CM4/CM4S
  DEVICE_ALT1_VARIANT := (32bit)
  DEVICE_DTS := \
	broadcom/bcm2709-rpi-2-b broadcom/bcm2710-rpi-2-b \
	broadcom/bcm2710-rpi-3-b broadcom/bcm2710-rpi-3-b-plus \
	broadcom/bcm2710-rpi-cm3 broadcom/bcm2711-rpi-4-b \
	broadcom/bcm2711-rpi-400 broadcom/bcm2711-rpi-cm4-io \
	broadcom/bcm2711-rpi-cm4 broadcom/bcm2711-rpi-cm4s \
	broadcom/bcm2710-rpi-zero-2 broadcom/bcm2710-rpi-zero-2-w \
	broadcom/bcm2710-rpi-cm0
  SUPPORTED_DEVICES := \
	rpi-2-b \
	rpi-3-a-plus \
	rpi-3-b rpi-3-b-plus \
	rpi-4-b rpi-400 \
	rpi-cm rpi-cm3 \
	rpi-zero-2 rpi-zero-2-w rpi-cm0 \
	raspberrypi,2-model-b raspberrypi,2-model-b-rev2 \
	raspberrypi,3-model-a-plus raspberrypi,3-model-b \
	raspberrypi,3-model-b-plus raspberrypi,3-compute-module \
	raspberrypi,compute-module-3 raspberrypi,400 \
	raspberrypi,4-compute-module raspberrypi,4-compute-module-s raspberrypi,4-model-b \
	raspberrypi,model-zero-2 raspberrypi,model-zero-2-w raspberrypi,0-compute-module
  DEVICE_PACKAGES := \
	$(WIFI_43430_FW) \
	$(WIFI_43455_FW) \
	$(BRCM_WIFI_DRIVER)
  IMAGE/sysupgrade.img.gz := boot-common | boot-2708 | boot-2711 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2708 | boot-2711 | sdcard-img | gzip
endef
ifeq ($(SUBTARGET),bcm2709)
  TARGET_DEVICES += rpi-2
endif

define Device/rpi-3
  DEVICE_MODEL := 3A+/3B/3B+/CM3/Zero2/Zero2W/CM0
  DEVICE_VARIANT := (64bit)
  DEVICE_ALT0_VENDOR := Raspberry Pi
  DEVICE_ALT0_MODEL := 2B-1.2
  DEVICE_ALT0_VARIANT := (64bit)
  KERNEL_IMG := kernel8.img
  DEVICE_DTS := \
	broadcom/bcm2710-rpi-2-b \
	broadcom/bcm2710-rpi-3-b broadcom/bcm2710-rpi-3-b-plus \
	broadcom/bcm2710-rpi-cm3 broadcom/bcm2710-rpi-cm0 \
	broadcom/bcm2710-rpi-zero-2 broadcom/bcm2710-rpi-zero-2-w
  SUPPORTED_DEVICES := \
	rpi-3-a-plus \
	rpi-3-b rpi-3-b-plus rpi-cm3 \
	rpi-zero-2 rpi-zero-2-w rpi-cm0 \
	raspberrypi,2-model-b-rev2 \
	raspberrypi,3-model-a-plus \
	raspberrypi,3-model-b raspberrypi,3-model-b-plus \
	raspberrypi,3-compute-module raspberrypi,compute-module-3 \
	raspberrypi,model-zero-2 raspberrypi,model-zero-2-w raspberrypi,0-compute-module
  DEVICE_PACKAGES := \
	$(WIFI_43430_FW) \
	$(WIFI_43455_FW) \
	$(BRCM_WIFI_DRIVER)
endef
ifeq ($(SUBTARGET),bcm2710)
  TARGET_DEVICES += rpi-3
endif

define Device/rpi-4
  DEVICE_MODEL := 4B/400/CM4/CM4S
  DEVICE_VARIANT := (64bit)
  KERNEL_IMG := kernel8.img
  DEVICE_DTS := \
	broadcom/bcm2711-rpi-4-b \
	broadcom/bcm2711-rpi-400 \
	broadcom/bcm2711-rpi-cm4-io \
	broadcom/bcm2711-rpi-cm4 \
	broadcom/bcm2711-rpi-cm4s
  SUPPORTED_DEVICES := \
	raspberrypi,400 \
	raspberrypi,4-compute-module \
	raspberrypi,4-compute-module-s \
	raspberrypi,4-model-b
  DEVICE_PACKAGES := \
	$(WIFI_43455_FW) \
	$(BRCM_WIFI_DRIVER) \
	$(LAN_DRIVERS)
  IMAGE/sysupgrade.img.gz := boot-common | boot-2711 | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | boot-2711 | sdcard-img | gzip
endef
ifeq ($(SUBTARGET),bcm2711)
  TARGET_DEVICES += rpi-4
endif

define Device/rpi-5
  DEVICE_MODEL := 5/500/CM5/CM5L
  KERNEL_IMG := kernel_2712.img
  DEVICE_DTS := \
	broadcom/bcm2712-d-rpi-5-b \
	broadcom/bcm2712-rpi-5-b \
	broadcom/bcm2712-rpi-500 \
	broadcom/bcm2712-rpi-cm5-cm4io \
	broadcom/bcm2712-rpi-cm5-cm5io \
	broadcom/bcm2712-rpi-cm5l-cm4io \
	broadcom/bcm2712-rpi-cm5l-cm5io \
	broadcom/bcm2712d0-rpi-5-b
  SUPPORTED_DEVICES := \
	raspberrypi,500 \
	raspberrypi,5-compute-module \
	raspberrypi,5-model-b
  DEVICE_PACKAGES := \
	$(WIFI_43455_FW) \
	$(BRCM_WIFI_DRIVER) \
	$(LAN_DRIVERS) \
	kmod-hwmon-pwmfan kmod-thermal
  IMAGE/sysupgrade.img.gz := boot-common | sdcard-img | gzip | append-metadata
  IMAGE/factory.img.gz := boot-common | sdcard-img | gzip
endef
ifeq ($(SUBTARGET),bcm2712)
  TARGET_DEVICES += rpi-5
endif

$(eval $(call BuildImage))
