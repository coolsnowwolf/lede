#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

NAND_BLOCKSIZE := 2048-128k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR:=0x8000
TARGET_DEVICES = linksys-audi linksys-viper dockstar goflexnet goflexhome iconnect pogo_e02 ib62x0 nsa310b nsa325 on100

UBI_OPTS := -m 2048 -p 128KiB -s 512
UBIFS_OPTS := -m 2048 -e 126KiB -c 4096

define Device/Default
  PROFILES := Default
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL := kernel-bin | append-dtb | uImage none
  KERNEL_NAME := zImage
  KERNEL_SUFFIX  := -uImage
  KERNEL_INSTALL := 1

  PAGESIZE := 2048
  SUBPAGESIZE := 512
  BLOCKSIZE := 128k
  IMAGES := sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar
endef

define Device/dockstar
  DEVICE_DTS := kirkwood-dockstar
  FILESYSTEMS := squashfs
  DEVICE_TITLE := Seagate FreeAgent Dockstar
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi
  KERNEL_IN_UBI := 1
endef

define Device/goflexnet
$(Device/dockstar)
  DEVICE_TITLE := Seagate GoFlexNet
  DEVICE_DTS := kirkwood-goflexnet
endef

define Device/goflexhome
$(Device/dockstar)
  DEVICE_TITLE := Seagate GoFlexHome
  DEVICE_DTS := kirkwood-goflexhome
endef

define Device/linksys-audi
  DEVICE_TITLE := Linksys EA3500 (Audi)
  DEVICE_PACKAGES := kmod-mwl8k swconfig wpad-mini
  DEVICE_DTS := kirkwood-linksys-audi
  KERNEL_SIZE := 2624k
  FILESYSTEMS := squashfs
  IMAGES += factory.bin
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

define Device/linksys-viper
  DEVICE_TITLE := Linksys E4200v2 / EA4500 (Viper)
  DEVICE_PACKAGES := kmod-mwl8k swconfig wpad-mini
  DEVICE_DTS := kirkwood-linksys-viper
  KERNEL_SIZE := 2688k
  FILESYSTEMS := squashfs
  IMAGES += factory.bin
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

define Device/iconnect
$(Device/dockstar)
  DEVICE_TITLE := Iomega Iconnect
  DEVICE_DTS := kirkwood-iconnect
endef

define Device/nsa310b
$(Device/dockstar)
  DEVICE_TITLE := ZyXEL NSA310b
  DEVICE_DTS := kirkwood-nsa310b
  DEVICE_PACKAGES := kmod-r8169 kmod-gpio-button-hotplug kmod-hwmon-lm85
endef

define Device/nsa325
$(Device/dockstar)
  DEVICE_TITLE := ZyXEL NSA325 (v1 and v2)
  DEVICE_DTS := kirkwood-nsa325
  DEVICE_PACKAGES := kmod-gpio-button-hotplug kmod-rtc-pcf8563 kmod-usb3
endef

define Device/on100
  DEVICE_TITLE := Cisco Systems ON100
  DEVICE_DTS := kirkwood-on100
#  DEVICE_PACKAGES := kmod-mvsdio kmod-usb-storage kmod-i2c-core kmod-i2c-mv64xxx kmod-ata-core kmod-btmrvl kmod-btmrvl-sdio kmod-libertas kmod-libertas-sdio
  DEVICE_PACKAGES := kmod-i2c-mv64xxx
  KERNEL_SIZE := 5376k
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  UBINIZE_OPTS := -E 5
endef

define Device/pogo_e02
$(Device/dockstar)
  DEVICE_TITLE := Cloud Engines Pogoplug E02
  DEVICE_DTS := kirkwood-pogo_e02
endef

define Device/ib62x0
$(Device/dockstar)
  DEVICE_TITLE := RaidSonic ICY BOX IB-NAS62x0
  DEVICE_DTS := kirkwood-ib62x0
endef

$(eval $(call BuildImage))
