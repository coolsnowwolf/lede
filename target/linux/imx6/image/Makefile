#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

#################################################
# Images
#################################################

DEVICE_VARS += MKUBIFS_OPTS UBOOT BOOT_SCRIPT

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
		$(MKUBIFS_OPTS) -c 8124 \
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

define Build/boot-scr
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
	-n '$(DEVICE_ID) OpenWrt bootscript' \
	-d ./bootscript-$(DEVICE_NAME) \
	$(BIN_DIR)/boot.scr
endef

define Build/imx6-sdcard
	rm -rf $@.boot
	mkdir -p $@.boot

	$(CP) $(STAGING_DIR_IMAGE)/$(UBOOT)-u-boot.img $@.boot/u-boot.img
	$(CP) $(IMAGE_KERNEL) $@.boot/uImage

	$(foreach dts,$(DEVICE_DTS), \
		$(CP) \
			$(DTS_DIR)/$(dts).dtb \
			$@.boot/;
	)

	mkimage -A arm -O linux -T script -C none -a 0 -e 0 \
		-n '$(DEVICE_ID) OpenWrt bootscript' \
		-d $(BOOT_SCRIPT) \
		$@.boot/boot.scr

	cp $@ $@.fs

	$(SCRIPT_DIR)/gen_image_generic.sh $@ \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) \
		$@.boot \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE) \
		$@.fs \
		1024

	dd if=$(STAGING_DIR_IMAGE)/$(UBOOT)-SPL of=$@ bs=1024 seek=1 conv=notrunc

	rm -rf $@.boot $@.fs
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

define Device/ventana
  DEVICE_TITLE := Gateworks Ventana family (normal NAND flash)
  DEVICE_DTS:= \
	imx6dl-gw51xx \
	imx6dl-gw52xx \
	imx6dl-gw53xx \
	imx6dl-gw54xx \
	imx6dl-gw551x \
	imx6dl-gw552x \
	imx6dl-gw553x \
	imx6dl-gw5904 \
	imx6q-gw51xx \
	imx6q-gw52xx \
	imx6q-gw53xx \
	imx6q-gw54xx \
	imx6q-gw5400-a \
	imx6q-gw551x \
	imx6q-gw552x \
	imx6q-gw553x \
	imx6q-gw5904
  DEVICE_PACKAGES := kmod-sky2 kmod-sound-core kmod-sound-soc-imx kmod-sound-soc-imx-sgtl5000 \
	kmod-can kmod-can-flexcan kmod-can-raw \
	kmod-hwmon-gsc \
	kmod-leds-gpio kmod-pps-gpio \
	kobs-ng
  KERNEL += | boot-overlay
  IMAGES := nand.ubi bootfs.tar.gz
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1).$$(2)
  UBINIZE_PARTS = boot=$$(KDIR_KERNEL_IMAGE).boot.ubifs=15
  BOOT_SCRIPT := bootscript-ventana
  IMAGE/nand.ubi := append-ubi
  IMAGE/bootfs.tar.gz := bootfs.tar.gz | install-dtb
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1)-$$(2)
  PAGESIZE := 2048
  BLOCKSIZE := 128k
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 124KiB
endef
TARGET_DEVICES += ventana

define Device/ventana-large
  $(Device/ventana)
  DEVICE_NAME := ventana
  DEVICE_TITLE := Gateworks Ventana family (large NAND flash)
  IMAGES := nand.ubi
  PAGESIZE := 4096
  BLOCKSIZE := 256k
  MKUBIFS_OPTS := -m $$(PAGESIZE) -e 248KiB
endef
TARGET_DEVICES += ventana-large

define Device/wandboard
  DEVICE_TITLE := Wandboard Dual
  DEVICE_DTS := imx6dl-wandboard
endef
TARGET_DEVICES += wandboard

define Device/cubox-i
  KERNEL := kernel-bin | install-dtb
  UBOOT := mx6cuboxi
  BOOT_SCRIPT = bootscript-cubox
  DEVICE_NAME := cubox
  DEVICE_TITLE := SolidRun CuBox-i
  DEVICE_PACKAGES := kmod-drm-imx kmod-drm-imx-hdmi kmod-usb-hid
  DEVICE_DTS := imx6q-cubox-i imx6dl-cubox-i imx6q-hummingboard imx6dl-hummingboard
  IMAGES := combined.bin
  FILESYSTEMS := squashfs
  IMAGE/combined.bin := append-rootfs | pad-extra 128k | imx6-sdcard
endef
TARGET_DEVICES += cubox-i

$(eval $(call BuildImage))
