# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/d-link_dgs-1210-52
  $(Device/d-link_dgs-1210)
  SOC := rtl8393
  DEVICE_MODEL := DGS-1210-52
endef
TARGET_DEVICES += d-link_dgs-1210-52

define Device/edgecore_ecs2100-52t
  $(Device/uimage-rt-loader)
  SOC := rtl8393
  DEVICE_VENDOR := Edgecore
  DEVICE_MODEL := ECS2100-52T
  IMAGE_SIZE := 30720k
  ECS_BOARD_ID := 01d8
  KERNEL := $$(KERNEL/rt-loader) | uImage none | edgecore-header
  KERNEL_INITRAMFS := $$(KERNEL/rt-loader) | uImage none | edgecore-header
  DEVICE_PACKAGES := \
	kmod-eeprom-at24 \
	kmod-hwmon-adt7470 \
	kmod-hwmon-lm75 \
	kmod-hwmon-pwmfan \
	kmod-thermal
endef
TARGET_DEVICES += edgecore_ecs2100-52t

define Device/edgecore_ecs4100-12ph
  $(Device/uimage-rt-loader)
  SOC := rtl8393
  DEVICE_VENDOR := Edgecore
  DEVICE_MODEL := ECS4100-12PH
  IMAGE_SIZE := 14336k
  DEVICE_PACKAGES := \
	kmod-eeprom-at24 \
	kmod-hwmon-adt7470 \
	kmod-hwmon-lm75 \
	kmod-thermal \
	realtek-poe
endef
TARGET_DEVICES += edgecore_ecs4100-12ph

define Device/hpe_1920-48g
  $(Device/hpe_1920)
  SOC := rtl8393
  DEVICE_MODEL := 1920-48G (JG927A)
  H3C_DEVICE_ID := 0x0001002a
endef
TARGET_DEVICES += hpe_1920-48g

define Device/hpe_1920-48g-poe
  $(Device/hpe_1920)
  SOC := rtl8393
  DEVICE_MODEL := 1920-48G-PoE (JG928A)
  DEVICE_PACKAGES += realtek-poe kmod-hwmon-gpiofan
  H3C_DEVICE_ID := 0x0001002b
endef
TARGET_DEVICES += hpe_1920-48g-poe

# When the factory image won't fit anymore, it can be removed.
# New installation will be performed booting the initramfs image from
# ram and then flashing the sysupgrade image from OpenWrt
define Device/netgear_gs750e
  SOC := rtl8393
  IMAGE_SIZE := 7552k
  FACTORY_SIZE := 6528k
  DEVICE_VENDOR := NETGEAR
  DEVICE_MODEL := GS750E
  UIMAGE_MAGIC := 0x174e4741
  IMAGES += factory.bix
  IMAGE/factory.bix := \
    append-kernel | \
    pad-to 64k | \
    append-rootfs | \
    pad-rootfs | \
    check-size $$$$(FACTORY_SIZE)
endef
TARGET_DEVICES += netgear_gs750e

define Device/panasonic_m48eg-pn28480k
  SOC := rtl8393
  IMAGE_SIZE := 16384k
  DEVICE_VENDOR := Panasonic
  DEVICE_MODEL := Switch-M48eG
  DEVICE_VARIANT := PN28480K
  DEVICE_PACKAGES := \
	kmod-hwmon-gpiofan \
	kmod-hwmon-lm75 \
	kmod-i2c-mux-pca954x \
	kmod-thermal
endef
TARGET_DEVICES += panasonic_m48eg-pn28480k

define Device/tplink_sg2452p-v4
  SOC := rtl8393
  KERNEL_SIZE := 6m
  IMAGE_SIZE := 26m
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := SG2452P
  DEVICE_VARIANT := v4
  DEVICE_PACKAGES := \
	  kmod-hwmon-gpiofan \
	  kmod-hwmon-tps23861
endef
TARGET_DEVICES += tplink_sg2452p-v4

define Device/zyxel_gs1900-48-a1
  $(Device/zyxel_gs1900)
  SOC := rtl8393
  DEVICE_MODEL := GS1900-48
  DEVICE_VARIANT := A1
  ZYXEL_VERS := AAHN
  SUPPORTED_DEVICES += zyxel,gs1900-48
endef
TARGET_DEVICES += zyxel_gs1900-48-a1

define Device/zyxel_gs1900-48hp-a1
  $(Device/zyxel_gs1900)
  SOC := rtl8393
  DEVICE_MODEL := GS1900-48HP
  DEVICE_VARIANT := A1
  ZYXEL_VERS := AAHO
  DEVICE_PACKAGES += realtek-poe
endef
TARGET_DEVICES += zyxel_gs1900-48hp-a1

define Device/zyxel_gs1920-24hp
ifeq ($(IB),)
  ARTIFACTS := loader.bin
  ARTIFACT/loader.bin := \
    rt-loader-standalone | \
    zynsig
endif
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := GS1920-24HP
  DEVICE_PACKAGES := kmod-hwmon-lm85 kmod-pse-realtek-mcu-i2c
  $(Device/rt-loader-bootbase)
endef

define Device/zyxel_gs1920-24hp-v1
  $(Device/zyxel_gs1920-24hp)
  SOC := rtl8392
  FLASH_ADDR := 0xb40c0000
  IMAGE_SIZE := 12144k
  DEVICE_VARIANT := v1
endef
TARGET_DEVICES += zyxel_gs1920-24hp-v1

define Device/zyxel_gs1920-24hp-v2
  $(Device/zyxel_gs1920-24hp)
  SOC := rtl8391
  FLASH_ADDR := 0xb4210000
  IMAGE_SIZE := 30720k
  DEVICE_VARIANT := v2
endef
TARGET_DEVICES += zyxel_gs1920-24hp-v2
