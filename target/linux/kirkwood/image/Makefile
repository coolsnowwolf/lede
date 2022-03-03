# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2009-2013 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR:=0x8000

# Some info about Ctera firmware:
# 1. It's simple tar file (GNU standard), but it must have ".firm" suffix.
# 2. It contains two images: kernel and romdisk. Both are required.
# 3. Every image has header and trailer file.
# 4. The struct of tar firmware is: header kernel trailer header romdisk trailer
# 5. In header file are some strings used to describe image. It was decoded from
#    factory image.
# 6. Version format in header file is restricted by Original FW.
# 7. Trailer file contains MD5 sum string of header and image file.
# 8. Firmware file must have <=24MB size.

define Build/ctera-firmware
	mkdir -p $@.tmp

	# Prepare header and trailer file for kernel
	echo "# CTera firmware information file" > $@.tmp/header
	echo "image_type=kernel" >> $@.tmp/header
	echo "arch=Kirkwood" >> $@.tmp/header
	echo "board=Any" >> $@.tmp/header
	echo "version=3.1.22.30669" >> $@.tmp/header
	echo "kernel_cmd=console=ttyS0,115200 earlyprintk" >> $@.tmp/header
	echo "date=$$(date $(if $(SOURCE_DATE_EPOCH),-d@$(SOURCE_DATE_EPOCH)))" \
		>> $@.tmp/header

	cp $@ $@.tmp/kernel

	echo "MD5=$$(cat $@.tmp/header $@.tmp/kernel | $(MKHASH) md5)" \
		> $@.tmp/trailer

	tar $(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-H gnu -C $@.tmp -cf $@.tar header kernel trailer

	# Prepare header and trailer file for fake romdisk
	echo "# CTera firmware information file" > $@.tmp/header
	echo "image_type=romdisk" >> $@.tmp/header
	echo "initrd=yes" >> $@.tmp/header
	echo "arch=Kirkwood" >> $@.tmp/header
	echo "board=Any" >> $@.tmp/header
	echo "version=3.1.22.30669" >> $@.tmp/header
	echo "date=$$(date $(if $(SOURCE_DATE_EPOCH),-d@$(SOURCE_DATE_EPOCH)))" \
		>> $@.tmp/header

	rm -f $@
	touch $@
	$(call Build/append-uImage-fakehdr, ramdisk)
	cp $@ $@.tmp/romdisk

	echo "MD5=$$(cat $@.tmp/header $@.tmp/romdisk | $(MKHASH) md5)" \
		> $@.tmp/trailer

	tar $(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-H gnu -C $@.tmp -rf $@.tar header romdisk trailer

	mv $@.tar $@
	rm -rf $@.tmp
endef

define Device/kernel-size-migration
  DEVICE_COMPAT_VERSION := 2.0
  DEVICE_COMPAT_MESSAGE := Partition design has changed compared to \
	older versions (up to 21.02) due to kernel size restrictions. \
	Upgrade via sysupgrade mechanism is not possible, so new \
	installation via factory style image is required.
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS = kirkwood-$(lastword $(subst _, ,$(1)))
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_NAME := zImage
  KERNEL_SUFFIX  := -uImage
  KERNEL_IN_UBI := 1

  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
  IMAGE/factory.bin := append-ubi
endef

define Device/checkpoint_l-50
  DEVICE_VENDOR := Check Point
  DEVICE_MODEL := L-50
  DEVICE_PACKAGES := kmod-ath9k kmod-gpio-button-hotplug kmod-mvsdio \
	kmod-rtc-s35390a kmod-usb-ledtrig-usbport wpad-basic-wolfssl
  IMAGES := sysupgrade.bin
endef
TARGET_DEVICES += checkpoint_l-50

define Device/cisco_on100
  DEVICE_VENDOR := Cisco Systems
  DEVICE_MODEL := ON100
  KERNEL_SIZE := 5376k
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  DEVICE_PACKAGES := kmod-mvsdio
  SUPPORTED_DEVICES += on100
endef
TARGET_DEVICES += cisco_on100

define Device/cloudengines_pogoe02
  DEVICE_VENDOR := Cloud Engines
  DEVICE_MODEL := Pogoplug E02
  DEVICE_DTS := kirkwood-pogo_e02
  SUPPORTED_DEVICES += pogo_e02
endef
TARGET_DEVICES += cloudengines_pogoe02

define Device/cloudengines_pogoplugv4
  DEVICE_VENDOR := Cloud Engines
  DEVICE_MODEL := Pogoplug V4
  DEVICE_DTS := kirkwood-pogoplug-series-4
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 kmod-mvsdio kmod-usb3 \
	kmod-gpio-button-hotplug
endef
TARGET_DEVICES += cloudengines_pogoplugv4

define Device/ctera_c200-v1
  DEVICE_VENDOR := Ctera
  DEVICE_MODEL := C200
  DEVICE_VARIANT := V1
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-gpio-button-hotplug \
	kmod-hwmon-lm63 kmod-rtc-s35390a kmod-usb-ledtrig-usbport
  KERNEL := kernel-bin | append-dtb | uImage none | ctera-firmware
  KERNEL_IN_UBI :=
  KERNEL_SUFFIX := -factory.firm
  IMAGES := sysupgrade.bin
endef
TARGET_DEVICES += ctera_c200-v1

define Device/endian_4i-edge-200
  DEVICE_VENDOR := Endian
  DEVICE_MODEL := 4i Edge 200
  DEVICE_ALT0_VENDOR := Endian
  DEVICE_ALT0_MODEL := UTM Mini Firewall
  DEVICE_PACKAGES := kmod-ath9k kmod-mvsdio wpad-basic-wolfssl
  KERNEL_SIZE := 4096k
  IMAGES := sysupgrade.bin
endef
TARGET_DEVICES += endian_4i-edge-200

define Device/globalscale_sheevaplug
  DEVICE_VENDOR := Globalscale
  DEVICE_MODEL := Sheevaplug
  DEVICE_PACKAGES := kmod-mvsdio
endef
TARGET_DEVICES += globalscale_sheevaplug

define Device/iom_iconnect-1.1
  DEVICE_VENDOR := Iomega
  DEVICE_MODEL := Iconnect
  DEVICE_DTS := kirkwood-iconnect
  SUPPORTED_DEVICES += iconnect
endef
TARGET_DEVICES += iom_iconnect-1.1

define Device/iom_ix2-200
  DEVICE_VENDOR := Iomega
  DEVICE_MODEL := StorCenter ix2-200
  DEVICE_DTS := kirkwood-iomega_ix2_200
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-hwmon-lm63
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16k
  KERNEL_SIZE := 3072k
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE_SIZE := 31744k
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | \
	check-size
endef
TARGET_DEVICES += iom_ix2-200

define Device/iptime_nas1
  DEVICE_VENDOR := ipTIME
  DEVICE_MODEL := NAS1
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-gpio-pca953x kmod-hwmon-drivetemp \
	kmod-hwmon-gpiofan kmod-usb-ledtrig-usbport -uboot-envtools
  KERNEL := $$(KERNEL) | iptime-naspkg nas1
  BLOCKSIZE := 256k
  IMAGE_SIZE := 15872k
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | \
	check-size | append-metadata
endef
TARGET_DEVICES += iptime_nas1

define Device/linksys
  DEVICE_VENDOR := Linksys
  DEVICE_PACKAGES := kmod-mwl8k wpad-basic-wolfssl kmod-gpio-button-hotplug
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

define Device/linksys_e4200-v2
  $(Device/linksys)
  $(Device/kernel-size-migration)
  DEVICE_MODEL := E4200
  DEVICE_VARIANT := v2
  KERNEL_SIZE := 3072k
  SUPPORTED_DEVICES += linksys,viper linksys-viper
endef
TARGET_DEVICES += linksys_e4200-v2

define Device/linksys_ea3500
  $(Device/linksys)
  $(Device/kernel-size-migration)
  DEVICE_MODEL := EA3500
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16k
  KERNEL_SIZE := 3072k
  SUPPORTED_DEVICES += linksys,audi linksys-audi
endef
TARGET_DEVICES += linksys_ea3500

define Device/linksys_ea4500
  $(Device/linksys)
  $(Device/kernel-size-migration)
  DEVICE_MODEL := EA4500
  KERNEL_SIZE := 3072k
  SUPPORTED_DEVICES += linksys,viper linksys-viper
endef
TARGET_DEVICES += linksys_ea4500

define Device/netgear_readynas-duo-v2
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := ReadyNAS Duo
  DEVICE_VARIANT := v2
  DEVICE_DTS := kirkwood-netgear_readynas_duo_v2
  KERNEL_IN_UBI :=
  IMAGES := sysupgrade.bin
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-hwmon-g762 kmod-rtc-rs5c372a kmod-usb3
endef
TARGET_DEVICES += netgear_readynas-duo-v2

define Device/raidsonic_ib-nas62x0
  DEVICE_VENDOR := RaidSonic
  DEVICE_MODEL := ICY BOX IB-NAS62x0
  DEVICE_DTS := kirkwood-ib62x0
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
  SUPPORTED_DEVICES += ib62x0
endef
TARGET_DEVICES += raidsonic_ib-nas62x0

define Device/seagate_blackarmor-nas220
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := Blackarmor NAS220
  DEVICE_PACKAGES := kmod-hwmon-adt7475 kmod-fs-ext4 kmod-ata-marvell-sata \
	mdadm kmod-gpio-button-hotplug
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16k
  UBINIZE_OPTS := -e 1
endef
TARGET_DEVICES += seagate_blackarmor-nas220

define Device/seagate_dockstar
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := FreeAgent Dockstar
  SUPPORTED_DEVICES += dockstar
endef
TARGET_DEVICES += seagate_dockstar

define Device/seagate_goflexnet
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := GoFlexNet
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
  SUPPORTED_DEVICES += goflexnet
endef
TARGET_DEVICES += seagate_goflexnet

define Device/seagate_goflexhome
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := GoFlexHome
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
  SUPPORTED_DEVICES += goflexhome
endef
TARGET_DEVICES += seagate_goflexhome

define Device/zyxel_nsa310b
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NSA310b
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-r8169 kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-hwmon-lm85
  SUPPORTED_DEVICES += nsa310b
endef
TARGET_DEVICES += zyxel_nsa310b

define Device/zyxel_nsa310s
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NSA310S
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 kmod-gpio-button-hotplug
endef
TARGET_DEVICES += zyxel_nsa310s

define Device/zyxel_nsa325
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NSA325
  DEVICE_VARIANT := v1/v2
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-rtc-pcf8563 kmod-usb3
  SUPPORTED_DEVICES += nsa325
endef
TARGET_DEVICES += zyxel_nsa325

$(eval $(call BuildImage))
