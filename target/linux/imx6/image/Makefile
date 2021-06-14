# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

#################################################
# Images
#################################################

DEVICE_VARS += MKUBIFS_OPTS UBOOT

define Build/boot-overlay
	rm -rf $@.boot
	mkdir -p $@.boot

	$(CP) $@ $@.boot/$(IMG_PREFIX)-uImage
	ln -sf $(IMG_PREFIX)-uImage $@.boot/uImage

	$(foreach dts,$(DEVICE_DTS), \
		$(CP) \
			$(DTS_DIR)/$(dts).dtb \
			$@.boot/$(IMG_PREFIX)-$(dts).dtb; \
		ln -sf \
			$(IMG_PREFIX)-$(dts).dtb \
			$@.boot/$(dts).dtb; \
	)
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
		-n '$(DEVICE_ID) OpenWrt bootscript' \
		-d ./bootscript-$(DEVICE_NAME) \
		$@.boot/6x_bootscript-$(DEVICE_NAME)

	$(STAGING_DIR_HOST)/bin/mkfs.ubifs \
		--space-fixup --compr=zlib --squash-uids \
		$(MKUBIFS_OPTS) -c 16248 \
		-o $@.boot.ubifs -d $@.boot

	$(TAR) -C $@.boot -cf $@.boot.tar .
endef

define Build/bootfs.tar.gz
	rm -rf $@.boot
	mkdir -p $@.boot

	$(TAR) -C $@.boot -xf $(IMAGE_KERNEL).boot.tar
	$(TAR) -C $@.boot \
		--numeric-owner --owner=0 --group=0 --transform "s,./,./boot/," \
		-czvf $@ .
endef

define Build/recovery-scr
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
	-n '$(DEVICE_ID) OpenWrt recovery bootscript' \
	-d ./recovery-$(DEVICE_NAME) $@
endef

define Build/imx6-combined-image-prepare
	rm -rf $@.boot
	mkdir -p $@.boot
endef

define Build/imx6-combined-image-clean
	rm -rf $@.boot $@.fs
endef

define Build/imx6-combined-image
	$(CP) $(IMAGE_KERNEL) $@.boot/uImage

	$(foreach dts,$(DEVICE_DTS), \
		$(CP) \
			$(DTS_DIR)/$(dts).dtb \
			$@.boot/;
	)

	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
		-n '$(DEVICE_ID) OpenWrt bootscript' \
		-d bootscript-$(DEVICE_NAME) \
		$@.boot/boot.scr

	cp $@ $@.fs

	$(SCRIPT_DIR)/gen_image_generic.sh $@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) \
		$@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$@.fs \
		1024
endef

define Build/imx6-sdcard
	$(Build/imx6-combined-image-prepare)

	$(CP) $(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.img $@.boot/u-boot.img
	$(Build/imx6-combined-image)
	dd if=$(STAGING_DIR_IMAGE)/$(UBOOT)-SPL of=$@ bs=1024 seek=1 conv=notrunc

	$(Build/imx6-combined-image-clean)
endef

define Build/apalis-emmc
	$(Build/imx6-combined-image-prepare)
	$(Build/imx6-combined-image)
	$(Build/imx6-combined-image-clean)
endef

#################################################
# Devices
#################################################

KERNEL_LOADADDR=0x10008000

define Device/Default
  PROFILES := Generic
  FILESYSTEMS := squashfs ext4
  KERNEL_INSTALL := 1
  KERNEL_SUFFIX := -uImage
  KERNEL_NAME := zImage
  KERNEL_PREFIX := $$(IMAGE_PREFIX)
  KERNEL := kernel-bin | uImage none
  IMAGES :=
endef

define Device/gateworks_ventana
  DEVICE_VENDOR := Gateworks
  DEVICE_MODEL := Ventana family
  DEVICE_VARIANT := normal NAND flash
  DEVICE_NAME := gateworks_ventana
  DEVICE_DTS:= \
	imx6dl-gw51xx \
	imx6dl-gw52xx \
	imx6dl-gw53xx \
	imx6dl-gw54xx \
	imx6dl-gw551x \
	imx6dl-gw552x \
	imx6dl-gw553x \
	imx6dl-gw5904 \
	imx6dl-gw5907 \
	imx6dl-gw5910 \
	imx6dl-gw5912 \
	imx6dl-gw5913 \
	imx6q-gw51xx \
	imx6q-gw52xx \
	imx6q-gw53xx \
	imx6q-gw54xx \
	imx6q-gw5400-a \
	imx6q-gw551x \
	imx6q-gw552x \
	imx6q-gw553x \
	imx6q-gw5904 \
	imx6q-gw5907 \
	imx6q-gw5910 \
	imx6q-gw5912 \
	imx6q-gw5913
  DEVICE_PACKAGES := kmod-sky2 kmod-sound-core kmod-sound-soc-imx \
	kmod-sound-soc-imx-sgtl5000 kmod-can kmod-can-flexcan kmod-can-raw \
	kmod-hwmon-gsc kmod-leds-gpio kmod-pps-gpio kobs-ng
  KERNEL += | boot-overlay
  IMAGES := nand.ubi bootfs.tar.gz dtb
  IMAGE/nand.ubi := append-ubi
  IMAGE/bootfs.tar.gz := bootfs.tar.gz
  IMAGE/dtb := install-dtb
  UBINIZE_PARTS = boot=$$(KDIR_KERNEL_IMAGE).boot.ubifs=15
  PAGESIZE := 2048
  BLOCKSIZE := 128k
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB
endef
TARGET_DEVICES += gateworks_ventana

define Device/gateworks_ventana-large
  $(Device/gateworks_ventana)
  DEVICE_VARIANT := large NAND flash
  IMAGES := nand.ubi
  PAGESIZE := 4096
  BLOCKSIZE := 256k
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 248KiB
endef
TARGET_DEVICES += gateworks_ventana-large

define Device/solidrun_cubox-i
  DEVICE_VENDOR := SolidRun
  DEVICE_MODEL := CuBox-i
  DEVICE_DTS := \
	imx6q-cubox-i \
	imx6dl-cubox-i \
	imx6q-hummingboard \
	imx6dl-hummingboard
  DEVICE_PACKAGES := kmod-drm-imx kmod-drm-imx-hdmi kmod-usb-hid
  UBOOT := mx6cuboxi
  KERNEL := kernel-bin
  KERNEL_SUFFIX := -zImage
  FILESYSTEMS := squashfs
  IMAGES := combined.bin dtb
  IMAGE/combined.bin := append-rootfs | pad-extra 128k | imx6-sdcard
  IMAGE/dtb := install-dtb
endef
TARGET_DEVICES += solidrun_cubox-i

define Device/toradex_apalis
  DEVICE_VENDOR := Toradex
  DEVICE_MODEL := Apalis family
  SUPPORTED_DEVICES := apalis,ixora apalis,eval
  DEVICE_DTS := \
	imx6q-apalis-eval \
	imx6q-apalis-ixora \
	imx6q-apalis-ixora-v1.1
  DEVICE_PACKAGES := \
	kmod-can kmod-can-flexcan kmod-can-raw \
	kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-pps-gpio kmod-rtc-ds1307
  FILESYSTEMS := squashfs
  IMAGES := combined.bin sysupgrade.bin
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1).$$(2)
  IMAGE/combined.bin := append-rootfs | pad-extra 128k | apalis-emmc
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  ARTIFACTS := recovery.scr
  ARTIFACT/recovery.scr := recovery-scr
endef
TARGET_DEVICES += toradex_apalis

define Device/wandboard_dual
  DEVICE_VENDOR := Wandboard
  DEVICE_MODEL := Dual
  DEVICE_DTS := imx6dl-wandboard
endef
TARGET_DEVICES += wandboard_dual

$(eval $(call BuildImage))
