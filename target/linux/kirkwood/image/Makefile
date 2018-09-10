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
  SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
endef

define Device/cisco_on100
  DEVICE_TITLE := Cisco Systems ON100
  DEVICE_DTS := kirkwood-on100
  DEVICE_PACKAGES := kmod-i2c-mv64xxx
  KERNEL_SIZE := 5376k
  KERNEL_IN_UBI := 0
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := on100
endef
TARGET_DEVICES += cisco_on100

define Device/cloudengines_pogoe02
  DEVICE_TITLE := Cloud Engines Pogoplug E02
  DEVICE_DTS := kirkwood-pogo_e02
  BOARD_NAME := pogo_e02
  SUPPORTED_DEVICES += pogo_e02
endef
TARGET_DEVICES += cloudengines_pogoe02

define Device/cloudengines_pogoplugv4
  DEVICE_TITLE := Cloud Engines Pogoplug V4
  DEVICE_DTS := kirkwood-pogoplug-series-4
  DEVICE_PACKAGES := kmod-usb3
endef
TARGET_DEVICES += cloudengines_pogoplugv4

define Device/iom_iconnect-1.1
  DEVICE_TITLE := Iomega Iconnect
  DEVICE_DTS := kirkwood-iconnect
  BOARD_NAME := iconnect
  SUPPORTED_DEVICES += iconnect
endef
TARGET_DEVICES += iom_iconnect-1.1

define Device/iom_ix2_200
  DEVICE_TITLE := Iomega StorCenter ix2-200
  DEVICE_DTS := kirkwood-iomega_ix2_200
  DEVICE_PACKAGES += kmod-gpio-button-hotplug kmod-i2c-mv64xxx kmod-hwmon-lm63
  DEVICE_TYPE:=nas
  PAGESIZE := 512
  SUBPAGESIZE := 256
  BLOCKSIZE := 16KiB
  KERNEL_SIZE := 3072k
  KERNEL_IN_UBI := 0
  UBINIZE_OPTS := -E 5  
  IMAGE_SIZE := 32505856
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | check-size $$$$(IMAGE_SIZE)
endef
TARGET_DEVICES += iom_ix2_200

define Device/linksys_audi
  DEVICE_TITLE := Linksys EA3500 (Audi)
  DEVICE_PACKAGES := kmod-mwl8k swconfig wpad-mini kmod-gpio-button-hotplug
  DEVICE_DTS := kirkwood-linksys-audi
  KERNEL_SIZE := 2624k
  KERNEL_IN_UBI := 0
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := linksys-audi
  SUPPORTED_DEVICES += linksys-audi
endef
TARGET_DEVICES += linksys_audi

define Device/linksys_viper
  DEVICE_TITLE := Linksys E4200v2 / EA4500 (Viper)
  DEVICE_PACKAGES := kmod-mwl8k swconfig wpad-mini kmod-gpio-button-hotplug
  DEVICE_DTS := kirkwood-linksys-viper
  KERNEL_SIZE := 2688k
  KERNEL_IN_UBI := 0
  UBINIZE_OPTS := -E 5
  IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi
  BOARD_NAME := linksys-viper
  SUPPORTED_DEVICES += linksys-viper
endef
TARGET_DEVICES += linksys_viper

define Device/raidsonic_ib-nas62x0
  DEVICE_TITLE := RaidSonic ICY BOX IB-NAS62x0
  DEVICE_DTS := kirkwood-ib62x0
  BOARD_NAME := ib62x0
  SUPPORTED_DEVICES += ib62x0
endef
TARGET_DEVICES += raidsonic_ib-nas62x0

define Device/seagate_dockstar
  DEVICE_DTS := kirkwood-dockstar
  DEVICE_TITLE := Seagate FreeAgent Dockstar
  BOARD_NAME := dockstar
  SUPPORTED_DEVICES += dockstar
endef
TARGET_DEVICES += seagate_dockstar

define Device/seagate_goflexnet
  DEVICE_TITLE := Seagate GoFlexNet
  DEVICE_DTS := kirkwood-goflexnet
  BOARD_NAME := goflexnet
  SUPPORTED_DEVICES += goflexnet
endef
TARGET_DEVICES += seagate_goflexnet

define Device/seagate_goflexhome
  DEVICE_TITLE := Seagate GoFlexHome
  DEVICE_DTS := kirkwood-goflexhome
  BOARD_NAME := goflexhome
  SUPPORTED_DEVICES += goflexhome
endef
TARGET_DEVICES += seagate_goflexhome

define Device/zyxel_nsa310b
  DEVICE_TITLE := ZyXEL NSA310b
  DEVICE_DTS := kirkwood-nsa310b
  DEVICE_PACKAGES := kmod-r8169 kmod-gpio-button-hotplug kmod-hwmon-lm85
  BOARD_NAME := nsa310b
endef
TARGET_DEVICES += zyxel_nsa310b

define Device/zyxel_nsa325
  DEVICE_TITLE := ZyXEL NSA325 (v1 and v2)
  DEVICE_DTS := kirkwood-nsa325
  DEVICE_PACKAGES := kmod-gpio-button-hotplug kmod-rtc-pcf8563 kmod-usb3
  BOARD_NAME := nsa325
endef
TARGET_DEVICES += zyxel_nsa325

$(eval $(call BuildImage))
