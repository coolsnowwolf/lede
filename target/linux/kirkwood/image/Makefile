#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR:=0x8000

define Device/Default
  PROFILES := Default
  DEVICE_DTS = $$(if $$(BOARD_NAME),kirkwood-$$(BOARD_NAME),)
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
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1)) $$(BOARD_NAME)
endef

define Device/cisco_on100
  DEVICE_VENDOR := Cisco Systems
  DEVICE_MODEL := ON100
  KERNEL_SIZE := 5376k
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := on100
  DEVICE_PACKAGES := kmod-mvsdio
endef
TARGET_DEVICES += cisco_on100

define Device/cloudengines_pogoe02
  DEVICE_VENDOR := Cloud Engines
  DEVICE_MODEL := Pogoplug E02
  BOARD_NAME := pogo_e02
endef
TARGET_DEVICES += cloudengines_pogoe02

define Device/cloudengines_pogoplugv4
  DEVICE_VENDOR := Cloud Engines
  DEVICE_MODEL := Pogoplug V4
  DEVICE_DTS := kirkwood-pogoplug-series-4
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 kmod-mvsdio kmod-usb3
endef
TARGET_DEVICES += cloudengines_pogoplugv4

define Device/iom_iconnect-1.1
  DEVICE_VENDOR := Iomega
  DEVICE_MODEL := Iconnect
  BOARD_NAME := iconnect
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

define Device/linksys_audi
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := EA3500 (Audi)
  DEVICE_PACKAGES := kmod-mwl8k wpad-basic kmod-gpio-button-hotplug
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16k
  KERNEL_SIZE := 2624k
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := linksys-audi
endef
TARGET_DEVICES += linksys_audi

define Device/linksys_viper
  DEVICE_VENDOR := Linksys
  DEVICE_MODEL := E4200v2 / EA4500 (Viper)
  DEVICE_PACKAGES := kmod-mwl8k wpad-basic kmod-gpio-button-hotplug
  KERNEL_SIZE := 2688k
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := linksys-viper
endef
TARGET_DEVICES += linksys_viper

define Device/raidsonic_ib-nas62x0
  DEVICE_VENDOR := RaidSonic
  DEVICE_MODEL := ICY BOX IB-NAS62x0
  BOARD_NAME := ib62x0
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
endef
TARGET_DEVICES += raidsonic_ib-nas62x0

define Device/seagate_dockstar
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := FreeAgent Dockstar
  BOARD_NAME := dockstar
endef
TARGET_DEVICES += seagate_dockstar

define Device/seagate_goflexnet
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := GoFlexNet
  BOARD_NAME := goflexnet
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
endef
TARGET_DEVICES += seagate_goflexnet

define Device/seagate_goflexhome
  DEVICE_VENDOR := Seagate
  DEVICE_MODEL := GoFlexHome
  BOARD_NAME := goflexhome
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
endef
TARGET_DEVICES += seagate_goflexhome

define Device/zyxel_nsa310b
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NSA310b
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-r8169 kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-hwmon-lm85
  BOARD_NAME := nsa310b
endef
TARGET_DEVICES += zyxel_nsa310b

define Device/zyxel_nsa325
  DEVICE_VENDOR := ZyXEL
  DEVICE_MODEL := NSA325
  DEVICE_VARIANT := v1/v2
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4 \
	kmod-gpio-button-hotplug kmod-rtc-pcf8563 kmod-usb3
  BOARD_NAME := nsa325
endef
TARGET_DEVICES += zyxel_nsa325

$(eval $(call BuildImage))
