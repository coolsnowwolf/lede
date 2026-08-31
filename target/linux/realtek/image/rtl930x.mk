# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/d-link_dgs-1250-28x
  SOC := rtl9301
  DEVICE_VENDOR := D-Link
  DEVICE_MODEL := DGS-1250-28X
  DEVICE_PACKAGES += kmod-hwmon-lm75
  IMAGE_SIZE := 24576k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += d-link_dgs-1250-28x

define Device/sirivision_sr-st3408f
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := Sirivision
  DEVICE_MODEL := SR-ST3408F
  DEVICE_PACKAGES := kmod-phy-realtek rtl826x-firmware
  IMAGE_SIZE := 13312k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += sirivision_sr-st3408f

define Device/hasivo_f1100w-4sx-4xgt-common
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := F1100W-4SX-4XGT
  DEVICE_ALT0_VENDOR := Hasivo
  DEVICE_ALT0_MODEL := F1100W-4SX-4XGT-SE
  DEVICE_PACKAGES := \
    kmod-hasivo-mcu-sensor kmod-hasivo-mcu-wdt kmod-mfd-hasivo-stc8 \
    kmod-phy-realtek rtl826x-firmware
  IMAGE_SIZE := 29696k
  $(Device/kernel-lzma)
endef

define Device/hasivo_f1100w-4sx-4xgt
  $(Device/hasivo_f1100w-4sx-4xgt-common)
endef
TARGET_DEVICES += hasivo_f1100w-4sx-4xgt

define Device/hasivo_f1100w-4sx-4xgt-512mb
  $(Device/hasivo_f1100w-4sx-4xgt-common)
  DEVICE_VARIANT := 512MB
  DEVICE_ALT0_VARIANT := 512MB
endef
TARGET_DEVICES += hasivo_f1100w-4sx-4xgt-512mb

define Device/hasivo_f1100wp-4sx-4xgt-common
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := F1100WP-4SX-4XGT
  DEVICE_ALT0_VENDOR := Hasivo
  DEVICE_ALT0_MODEL := F1100WP-4SX-4XGT-SE
  DEVICE_PACKAGES := \
    kmod-hasivo-mcu-sensor kmod-hasivo-mcu-wdt kmod-mfd-hasivo-stc8 \
    kmod-phy-realtek kmod-pse-hasivo-hs104 rtl826x-firmware
  IMAGE_SIZE := 29696k
  $(Device/kernel-lzma)
endef

define Device/hasivo_f1100wp-4sx-4xgt
  $(Device/hasivo_f1100wp-4sx-4xgt-common)
endef
TARGET_DEVICES += hasivo_f1100wp-4sx-4xgt

define Device/hasivo_f1100wp-4sx-4xgt-512mb
  $(Device/hasivo_f1100wp-4sx-4xgt-common)
  DEVICE_VARIANT := 512MB
  DEVICE_ALT0_VARIANT := 512MB
endef
TARGET_DEVICES += hasivo_f1100wp-4sx-4xgt-512mb

define Device/hasivo_s1100w-8xgt-se
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S1100W-8XGT-SE
  IMAGE_SIZE := 12288k
  DEVICE_PACKAGES := rtl826x-firmware
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s1100w-8xgt-se

define Device/hasivo_s1100wp-8gt-se
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S1100WP-8GT-SE
  DEVICE_PACKAGES := kmod-pse-hasivo-hs104 kmod-mfd-hasivo-stc8
  IMAGE_SIZE := 12288k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s1100wp-8gt-se

define Device/hasivo_s1100wp-8xgt-se
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S1100WP-8XGT-SE
  IMAGE_SIZE := 12288k
  DEVICE_PACKAGES := rtl826x-firmware kmod-pse-hasivo-hs104 kmod-mfd-hasivo-stc8
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s1100wp-8xgt-se

define Device/hasivo_s600wp-5gt-2s-plus
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S600WP-5GT-2S+
  DEVICE_PACKAGES := kmod-pse-hasivo-hs104 kmod-mfd-hasivo-stc8
  IMAGE_SIZE := 29696k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s600wp-5gt-2s-plus

define Device/hasivo_s600wp-5gt-2sx-se
  SOC := rtl9303
  DEVICE_VENDOR := Hasivo
  DEVICE_MODEL := S600WP-5GT-2SX-SE
  DEVICE_PACKAGES := kmod-pse-hasivo-hs104 kmod-mfd-hasivo-stc8
  IMAGE_SIZE := 12288k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += hasivo_s600wp-5gt-2sx-se

define Device/horaco_zx-swtgw2c8f
  SOC := rtl9303
  UIMAGE_MAGIC := 0x83800000
  DEVICE_VENDOR := Horaco
  DEVICE_MODEL := ZX-SWTGW2C8F
  IMAGE_SIZE := 12288k
  $(Device/kernel-lzma)
  IMAGES += factory.bix
  IMAGE/factory.bix := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size
endef
TARGET_DEVICES += horaco_zx-swtgw2c8f

define Device/nicgiga_s100-0800s-m
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93030000
  DEVICE_VENDOR := NicGiga
  DEVICE_MODEL := S100-0800S-M
  DEVICE_PACKAGES := kmod-gpio-pca953x
  IMAGE_SIZE := 29696k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += nicgiga_s100-0800s-m

define Device/plasmacloud-common
  SOC := rtl9302
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := Plasma Cloud
  DEVICE_PACKAGES := poemgr
  IMAGE_SIZE := 15872k
  BLOCKSIZE := 64k
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma | pad-to $$(BLOCKSIZE)
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | append-rootfs | pad-rootfs | check-size
  IMAGE/sysupgrade.bin := append-rootfs | pad-rootfs | sysupgrade-tar rootfs=$$$$@ | append-metadata
endef

define Device/plasmacloud_mcx3
  $(Device/plasmacloud-common)
  DEVICE_MODEL := MCX3
endef
TARGET_DEVICES += plasmacloud_mcx3

define Device/plasmacloud_psx8
  $(Device/plasmacloud-common)
  DEVICE_MODEL := PSX8
endef
TARGET_DEVICES += plasmacloud_psx8

define Device/plasmacloud_psx10
  $(Device/plasmacloud-common)
  DEVICE_MODEL := PSX10
endef
TARGET_DEVICES += plasmacloud_psx10

define Device/tplink_tl-st1008f-v2
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93030000
  DEVICE_VENDOR := TP-Link
  DEVICE_MODEL := TL-ST1008F
  DEVICE_VARIANT := v2.0
  DEVICE_PACKAGES := kmod-gpio-pca953x
  SUPPORTED_DEVICES += tplink,tl-st1008f,v2
  IMAGE_SIZE := 31808k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += tplink_tl-st1008f-v2

define Device/ubnt_usw-aggregation
  SOC := rtl9303
  DEVICE_VENDOR := Ubiquiti
  DEVICE_MODEL := UniFi USW Aggregation
  IMAGE_SIZE := 14464k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += ubnt_usw-aggregation

define Device/vimin_vm-s100-0800ms
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := Vimin
  DEVICE_MODEL := VM-S100-0800MS
  IMAGE_SIZE := 13312k
  $(Device/kernel-lzma)
endef
TARGET_DEVICES += vimin_vm-s100-0800ms

define Device/xikestor_sks7300-4x4t
  SOC := rtl9303
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS7300-4X4T
  DEVICE_PACKAGES := kmod-hwmon-lm75
  KERNEL_SIZE:= 8192k
  IMAGE_SIZE := 28160k
  KERNEL := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    xikestor-sks7300-img
  KERNEL_INITRAMFS := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    xikestor-sks7300-img
  IMAGE/sysupgrade.bin := \
    append-kernel | \
    pad-to 8192k | \
    append-rootfs | \
    pad-rootfs | \
    check-size | \
    append-metadata
endef
TARGET_DEVICES += xikestor_sks7300-4x4t

define Device/xikestor_sks8300-8t
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS8300-8T
  DEVICE_PACKAGES := kmod-hwmon-lm75 rtl826x-firmware
  IMAGE_SIZE := 20480k
  $(Device/kernel-lzma)
  IMAGE/sysupgrade.bin := \
    pad-extra 16 | \
    append-kernel | \
    pad-to 64k | \
    append-rootfs | \
    pad-rootfs | \
    check-size | \
    append-metadata
endef
TARGET_DEVICES += xikestor_sks8300-8t

define Device/xikestor_sks8300-8x
  SOC := rtl9303
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS8300-8X
  DEVICE_ALT0_VENDOR := ONTi
  DEVICE_ALT0_MODEL := ONT-S508CL-8S
  BLOCKSIZE := 64k
  KERNEL_SIZE := 8192k
  IMAGE_SIZE := 30720k
  IMAGE/sysupgrade.bin := pad-extra 256 | append-kernel | xikestor-nosimg | \
	jffs2 nos.img -e 4KiB -x lzma | pad-to $$$$(KERNEL_SIZE) | \
	append-rootfs | pad-rootfs | append-metadata | check-size
endef
TARGET_DEVICES += xikestor_sks8300-8x

define Device/xikestor_sks8300-12e2t2x
  SOC := rtl9302
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS8300-12E2T2X
  DEVICE_PACKAGES := rtl826x-firmware
  IMAGE_SIZE := 20480k
  $(Device/kernel-lzma)
  IMAGE/sysupgrade.bin := \
    pad-extra 16 | \
    append-kernel | \
    pad-to 64k | \
    append-rootfs | \
    pad-rootfs | \
    check-size | \
    append-metadata
endef
TARGET_DEVICES += xikestor_sks8300-12e2t2x

define Device/xikestor_sks8310-8x
  SOC := rtl9303
  UIMAGE_MAGIC := 0x93000000
  DEVICE_VENDOR := XikeStor
  DEVICE_MODEL := SKS8310-8X
  DEVICE_PACKAGES := kmod-hwmon-lm75
  IMAGE_SIZE := 20480k
  $(Device/kernel-lzma)
  IMAGE/sysupgrade.bin := \
    pad-extra 16 | \
    append-kernel | \
    pad-to 64k | \
    append-rootfs | \
    pad-rootfs | \
    check-size | \
    append-metadata
endef
TARGET_DEVICES += xikestor_sks8310-8x

define Device/zyxel_xgs1010-12-a1
  $(Device/zyxel_xgs1010-12)
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += zyxel_xgs1010-12-a1

define Device/zyxel_xgs1010-12-b1
  $(Device/zyxel_xgs1010-12)
  DEVICE_VARIANT := B1
endef
TARGET_DEVICES += zyxel_xgs1010-12-b1

define Device/zyxel_xgs1210-12-a1
  $(Device/zyxel_xgs1210-12)
  SUPPORTED_DEVICES += zyxel,xgs1210-12
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += zyxel_xgs1210-12-a1

define Device/zyxel_xgs1210-12-b1
  $(Device/zyxel_xgs1210-12)
  DEVICE_VARIANT := B1
endef
TARGET_DEVICES += zyxel_xgs1210-12-b1

define Device/zyxel_xgs1250-12-common
  SOC := rtl9302
  UIMAGE_MAGIC := 0x93001250
  ZYXEL_VERS := ABWE
  DEVICE_VENDOR := Zyxel
  DEVICE_MODEL := XGS1250-12
  DEVICE_PACKAGES := kmod-hwmon-gpiofan kmod-thermal
  IMAGE_SIZE := 13312k
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	rt-compress | \
	rt-loader | \
	uImage none
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	rt-compress | \
	zyxel-vers | \
	rt-loader | \
	uImage none
endef

define Device/zyxel_xgs1250-12-a1
  $(Device/zyxel_xgs1250-12-common)
  SUPPORTED_DEVICES += zyxel,xgs1250-12
  DEVICE_VARIANT := A1
endef
TARGET_DEVICES += zyxel_xgs1250-12-a1

define Device/zyxel_xgs1250-12-b1
  $(Device/zyxel_xgs1250-12-common)
  DEVICE_VARIANT := B1
  DEVICE_PACKAGES += rtl826x-firmware
endef
TARGET_DEVICES += zyxel_xgs1250-12-b1

define Device/zyxel_xgs1930-28hp
  SOC := rtl9301
  DEVICE_MODEL := XGS1930-28HP
  FLASH_ADDR := 0xb4260000
  IMAGE_SIZE := 30336k
  ZYNFW_ALIGN := 0x10000
  $(Device/zyxel_zynos)
  DEVICE_PACKAGES += kmod-hwmon-gpiofan kmod-pse-realtek-mcu-i2c
endef
TARGET_DEVICES += zyxel_xgs1930-28hp

define Device/zyxel_xmg1915
  SOC := rtl9302
  FLASH_ADDR := 0xb4270000
  IMAGE_SIZE := 30336k
  ZYNFW_ALIGN := 0x10000
  $(Device/zyxel_zynos)
endef

define Device/zyxel_xmg1915-10e
  DEVICE_MODEL := XMG1915-10E
  $(Device/zyxel_xmg1915)
endef
TARGET_DEVICES += zyxel_xmg1915-10e

define Device/zyxel_xmg1915-10ep
  DEVICE_MODEL := XMG1915-10EP
  $(Device/zyxel_xmg1915)
  DEVICE_PACKAGES += kmod-pse-realtek-mcu-uart
endef
TARGET_DEVICES += zyxel_xmg1915-10ep
