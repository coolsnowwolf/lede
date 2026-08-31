# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2021 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

# Cook a Linksys NSLU2 etc image
define Build/linksys-ixp425-image
	touch $@.null-initrd
	$(TOPDIR)/scripts/slugimage.pl -L $(STAGING_DIR_IMAGE)/apex/apex-$(1)-armeb.bin -k $@ -r $@.null-initrd -p -o $@.new
	mv $@.new $@
endef

define Build/freecom-image
	mkdir -p $@.tmptar
	# Add kernel
	cp $@ $@.tmptar/zImage
	cd $@.tmptar && tar -c -j -f $@.new --numeric-owner --owner=0 --group=0 *
	rm -rf $@.tmptar
	encode_crc $@.new $@
	rm -f $@.new
endef

# Build sysupgrade image
define BuildFirmware/Generic
	dd if=$(KDIR)/zImage of=$(KDIR)/zImage.pad bs=64k conv=sync; \
	dd if=$(KDIR)/root.$(1) of=$(KDIR)/root.$(1).pad bs=128k conv=sync; \
	sh $(TOPDIR)/scripts/combined-image.sh \
		$(KDIR)/zImage.pad \
		$(KDIR)/root.$(1).pad \
		$(BIN_DIR)/$(IMG_PREFIX)-$(patsubst jffs2-%,jffs2,$(patsubst squashfs-%,squashfs,$(1)))-sysupgrade.bin
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call BuildFirmware/Generic,$(1))
endef

define Device/Default
	PROFILES := Default
	DEVICE_DTS_DIR = $$(DTS_DIR)/intel/ixp
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_NAME := zImage
	KERNEL := kernel-bin | append-dtb
	BLOCKSIZE := 128k
endef

define Device/actiontec_mi424wr_ac
	DEVICE_VENDOR := Actiontec
	DEVICE_MODEL := MI424WR rev A/C
	# Only 8 MB of Flash so not building by default
	DEFAULT := n
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-dsa-ks8995
	DEVICE_DTS := intel-ixp42x-actiontec-mi424wr-ac
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += actiontec_mi424wr_ac

define Device/actiontec_mi424wr_d
	DEVICE_VENDOR := Actiontec
	DEVICE_MODEL := MI424WR rev D
	# Only 8 MB of Flash so not building by default
	DEFAULT := n
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-dsa-ks8995
	DEVICE_DTS := intel-ixp42x-actiontec-mi424wr-d
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += actiontec_mi424wr_d

define Device/dlink_dsm_g600_a
	DEVICE_VENDOR := D-Link
	DEVICE_MODEL := DSM G600 A
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-pcf8563 kmod-via-velocity kmod-ata-artop kmod-ath5k wpad-basic-mbedtls
	DEVICE_DTS := intel-ixp42x-dlink-dsm-g600
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += dlink_dsm_g600_a

define Device/freecom_fsg_3
	DEVICE_VENDOR := Freecom
	DEVICE_MODEL := FSG-3
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-isl1208 kmod-ath5k wpad-basic-mbedtls
	# Only 4 MB of Flash so not building by default
	DEFAULT := n
	DEVICE_DTS := intel-ixp42x-freecom-fsg-3
	KERNEL := kernel-bin | append-dtb
	IMAGES := factory.bin
	# This has to boot from harddisk so just append the kernel
	IMAGE/factory.bin := append-kernel | freecom-image
endef
TARGET_DEVICES += freecom_fsg_3

define Device/gateworks_avila
	DEVICE_VENDOR := Gateworks
	DEVICE_MODEL := Avila GW2348-4
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-ds1672 kmod-eeprom-at24 kmod-hwmon-ad7418
	DEVICE_DTS := intel-ixp42x-gateworks-gw2348
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += gateworks_avila

define Device/gateworks_cambria
	DEVICE_VENDOR := Gateworks
	DEVICE_MODEL := Cambria GW2358-4
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-ds1672 kmod-eeprom-at24 kmod-hwmon-ad7418
	DEVICE_DTS := intel-ixp43x-gateworks-gw2358
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += gateworks_cambria

define Device/iomega_nas100d
	DEVICE_VENDOR := Iomega
	DEVICE_MODEL := NAS100d
	# USB2 is compiled in and needs no package
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-pcf8563
	DEVICE_DTS := intel-ixp42x-iomega-nas100d
	KERNEL := kernel-bin | append-dtb
	IMAGES := factory.bin
	# This has to boot from harddisk so just append the kernel
	IMAGE/factory.bin := append-kernel | linksys-ixp425-image "nas100d"
endef
TARGET_DEVICES += iomega_nas100d

define Device/linksys_nslu2
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := NSLU2
	# USB2 is compiled in and needs no package
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-x1205
	# Only 32 MB of RAM so not building by default
	DEFAULT := n
	DEVICE_DTS := intel-ixp42x-linksys-nslu2
	KERNEL := kernel-bin | append-dtb
	IMAGES := factory.bin
	# This has to boot from harddisk so just append the kernel
	IMAGE/factory.bin := append-kernel | linksys-ixp425-image "nslu2"
endef
TARGET_DEVICES += linksys_nslu2

define Device/netgear_wg302v1
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WG302 v1
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-phy-amd kmod-ath5k \
		wpad-basic-mbedtls kmod-input-gpio-keys-polled
	# Only 32 MB of RAM and small flash so not building by default
	DEFAULT := n
	DEVICE_DTS := intel-ixp42x-netgear-wg302v1
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
	IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | pad-to 128k | append-metadata
endef
TARGET_DEVICES += netgear_wg302v1

define Device/usrobotics_usr8200
	DEVICE_VENDOR := USRobotics
	DEVICE_MODEL := USR8200
	# USB2 is compiled in and needs no package
	DEVICE_PACKAGES := ixp4xx-microcode-ethernet kmod-rtc-r7301 kmod-firewire kmod-firewire-ohci kmod-dsa-mv88e6060
	DEVICE_DTS := intel-ixp42x-usrobotics-usr8200
	KERNEL := kernel-bin | append-dtb
	IMAGES := kernel.bin rootfs.bin
	IMAGE/kernel.bin := append-kernel
	IMAGE/rootfs.bin := append-rootfs | pad-rootfs | pad-to 128k
endef
TARGET_DEVICES += usrobotics_usr8200

$(eval $(call BuildImage))
