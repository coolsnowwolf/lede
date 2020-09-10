#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR:=0x8000

define Device/dsa-migration
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA \
	(early adopters with DSA already set up may just force-flash keeping existing config)
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
  SUPPORTED_DEVICES = $(subst _,$(comma),$(1))
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

define Device/linksys
  DEVICE_VENDOR := Linksys
  DEVICE_PACKAGES := kmod-mwl8k wpad-basic-wolfssl kmod-gpio-button-hotplug
  KERNEL_IN_UBI :=
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
endef

define Device/linksys_e4200-v2
  $(Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := E4200
  DEVICE_VARIANT := v2
  KERNEL_SIZE := 2688k
  SUPPORTED_DEVICES += linksys,viper linksys-viper
endef
TARGET_DEVICES += linksys_e4200-v2

define Device/linksys_ea3500
  $(Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := EA3500
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16k
  KERNEL_SIZE := 2624k
  SUPPORTED_DEVICES += linksys,audi linksys-audi
endef
TARGET_DEVICES += linksys_ea3500

define Device/linksys_ea4500
  $(Device/linksys)
  $(Device/dsa-migration)
  DEVICE_MODEL := EA4500
  KERNEL_SIZE := 2688k
  SUPPORTED_DEVICES += linksys,viper linksys-viper
endef
TARGET_DEVICES += linksys_ea4500

define Device/raidsonic_ib-nas62x0
  DEVICE_VENDOR := RaidSonic
  DEVICE_MODEL := ICY BOX IB-NAS62x0
  DEVICE_DTS := kirkwood-ib62x0
  DEVICE_PACKAGES := kmod-ata-marvell-sata kmod-fs-ext4
  SUPPORTED_DEVICES += ib62x0
endef
TARGET_DEVICES += raidsonic_ib-nas62x0

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
