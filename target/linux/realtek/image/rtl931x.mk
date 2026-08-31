# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/hasivo_f5800w-12s-plus
  SOC := rtl9313
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := F5800W-12S+
  IMAGE_SIZE := 29696k
  DEVICE_PACKAGES := kmod-hasivo-mcu-wdt kmod-hasivo-mcu-sensor kmod-rtc-pcf8563
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_f5800w-12s-plus

define Device/hasivo_s1300wp-8xgt-4s-plus
  SOC := rtl9313
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S1300WP-8XGT-4S+
  IMAGE_SIZE := 29696k
  DEVICE_PACKAGES := kmod-phy-realtek kmod-rtc-pcf8563 rtl826x-firmware \
		     kmod-hasivo-mcu-wdt kmod-hasivo-mcu-sensor kmod-pse-hasivo-hs104 \
		     kmod-mfd-hasivo-stc8
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s1300wp-8xgt-4s-plus

define Device/mokerlink_10gt080m
  SOC := rtl9313
  DEVICE_VENDOR := MokerLink
  DEVICE_MODEL := 10GT080M
  IMAGE_SIZE := 14336k
  DEVICE_PACKAGES := kmod-phy-realtek rtl826x-firmware
  UIMAGE_MAGIC := 0x83800000
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += mokerlink_10gt080m

define Device/plasmacloud-common
  SOC := rtl9312
  UIMAGE_MAGIC := 0x93100000
  DEVICE_VENDOR := Plasma Cloud
  IMAGE_SIZE := 13376k
  BLOCKSIZE := 64k
  DEVICE_PACKAGES := kmod-hwmon-adt7475
  KERNEL_INITRAMFS := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    uImage lzma
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma | pad-to $$(BLOCKSIZE)
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | append-rootfs | pad-rootfs | check-size
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/plasmacloud_esx28
  $(Device/plasmacloud-common)
  DEVICE_MODEL := ESX28
endef
TARGET_DEVICES += plasmacloud_esx28

define Device/plasmacloud_psx28
  $(Device/plasmacloud-common)
  DEVICE_MODEL := PSX28
  DEVICE_PACKAGES += kmod-pse-realtek-mcu-i2c
endef
TARGET_DEVICES += plasmacloud_psx28

define Device/ubnt_usw-pro-xg-8-poe
  SOC := rtl9313
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi USW Pro XG 8 PoE
  IMAGE_SIZE := 30272k
  DEVICE_PACKAGES := rtl826x-firmware kmod-hwmon-adt7475 kmod-pse-realtek-mcu-i2c
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += ubnt_usw-pro-xg-8-poe

define Device/xikestor_sks8300-12x-v1
  SOC := rtl9313
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS8300-12X
  DEVICE_VARIANT := V1
  BLOCKSIZE := 64k
  KERNEL_SIZE := 8192k
  IMAGE_SIZE := 30720k
  IMAGE/sysupgrade.bin := pad-extra 256 | append-kernel | xikestor-nosimg | \
        jffs2 nos.img -e 4KiB -x lzma | pad-to $$$$(KERNEL_SIZE) | \
        append-rootfs | pad-rootfs | append-metadata | check-size
endef
TARGET_DEVICES += xikestor_sks8300-12x-v1

define Device/zyxel_xs1930
  SOC := rtl9313
  DEVICE_PACKAGES := kmod-hwmon-lm85 kmod-hwmon-gpiofan
  FLASH_ADDR := 0xb4280000
  IMAGE_SIZE := 31808k
  ZYNFW_ALIGN := 0x10000
  $(Device/zyxel_zynos)
endef

define Device/zyxel_xs1930-10
  DEVICE_MODEL := XS1930-10
  $(Device/zyxel_xs1930)
endef
TARGET_DEVICES += zyxel_xs1930-10

define Device/zyxel_xs1930-12f
  DEVICE_MODEL := XS1930-12F
  $(Device/zyxel_xs1930)
endef
TARGET_DEVICES += zyxel_xs1930-12f

define Device/zyxel_xs1930-12hp
  DEVICE_MODEL := XS1930-12HP
  $(Device/zyxel_xs1930)
  DEVICE_PACKAGES += kmod-pse-realtek-mcu-i2c
endef
TARGET_DEVICES += zyxel_xs1930-12hp
