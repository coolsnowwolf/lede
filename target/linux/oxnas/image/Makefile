#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 126KiB -c 4096

DEVICE_VARS += DTS UBIFS_OPTS

KERNEL_LOADADDR := 0x60008000

define Build/ubootable
	(dd if="$(STAGING_DIR_IMAGE)/u-boot.bin" bs=128k conv=sync; \
	 dd if="$@" bs=128k conv=sync ) >> $@.new
	@mv "$@.new" "$@"
endef

define Device/Default
  KERNEL_DEPENDS = $$(wildcard $$(DTS_DIR)/ox820-$$(DTS).dts)
  KERNEL = kernel-bin | lzma | fit lzma $$(DTS_DIR)/ox820-$$(DTS).dtb
  KERNEL_NAME := zImage
  KERNEL_SUFFIX := -uImage
  KERNEL_INSTALL := 1
  KERNEL_INITRAMFS = kernel-bin | lzma | fit lzma $$(DTS_DIR)/ox820-$$(DTS).dtb | ubootable
  KERNEL_INITRAMFS_PREFIX = $$(IMAGE_PREFIX)-u-boot-initramfs
  KERNEL_INITRAMFS_SUFFIX := .bin
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  SUBPAGESIZE := 512
  FILESYSTEMS := squashfs ubifs
  PROFILES = Default $$(DTS)
  IMAGES := ubinized.bin sysupgrade.tar
  IMAGE/ubinized.bin := append-ubi
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
  KERNEL_IN_UBI := 1
  UBOOTENV_IN_UBI := 1
endef

define Device/akitio
  DTS := akitio
  DEVICE_TITLE := Akitio MyCloud mini / Silverstone DC01
  DEVICE_PACKAGES := kmod-i2c-gpio kmod-rtc-ds1307
endef
TARGET_DEVICES += akitio

define Build/omninas-factory
	rm -rf $@.tmp $@.dummy $@.dummy.gz
	mkdir -p $@.tmp
	$(CP) $@ $@.tmp/uImage
	dd if=/dev/zero bs=64k count=4 of=$@.dummy
	gzip $@.dummy
	mkimage -A arm -T ramdisk -C gzip -n "dummy" \
		-d $@.dummy.gz \
		$@.tmp/rdimg.gz
	echo 2.35.20140102 > $@.tmp/version ; echo >> $@.tmp/version
	chmod 0744 $@.tmp/*
	$(TAR) -C $@.tmp -czvf $@ \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") .
endef

define Build/encrypt-3des
	openssl enc -des3 -a -k $(1) -in $@ -out $@.new && mv $@.new $@
endef

define Device/kd20
  DEVICE_DTS := ox820-kd20
  DEVICE_TITLE := Shuttle KD20
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_INITRAMFS_PREFIX = $$(IMAGE_PREFIX)-factory
  KERNEL_INITRAMFS_SUFFIX := .tar.gz
  KERNEL_INITRAMFS = kernel-bin | append-dtb | uImage none | omninas-factory | encrypt-3des sohmuntitnlaes
  KERNEL_IMAGE := zImage
  DEVICE_PACKAGES := kmod-usb3 kmod-i2c-gpio kmod-rtc-pcf8563 kmod-gpio-beeper \
                     kmod-hwmon-core kmod-hwmon-gpiofan \
                     kmod-md-mod kmod-md-raid0 kmod-md-raid1 kmod-fs-ext4 kmod-fs-xfs
  KERNEL_IN_UBI := 
  UBOOTENV_IN_UBI := 
endef
TARGET_DEVICES += kd20

define Device/pogoplug-pro
  DTS := pogoplug-pro
  DEVICE_TITLE := Cloud Engines Pogoplug Pro (with mPCIe)
endef
TARGET_DEVICES += pogoplug-pro

define Device/pogoplug-v3
  DTS := pogoplug-v3
  DEVICE_TITLE := Cloud Engines Pogoplug V3 (no mPCIe)
endef
TARGET_DEVICES += pogoplug-v3

define Device/stg212
  DTS := stg212
  DEVICE_TITLE := MitraStar STG-212
endef
TARGET_DEVICES += stg212

VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux
UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage

$(eval $(call BuildImage))
