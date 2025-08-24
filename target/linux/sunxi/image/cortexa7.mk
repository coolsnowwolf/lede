# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013-2019 OpenWrt.org
# Copyright (C) 2016 Yousong Zhou

KERNEL_LOADADDR:=0x40008000

define Device/cubietech_cubieboard2
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Cubietech
  DEVICE_MODEL := Cubieboard2
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SOC := sun7i-a20
endef
TARGET_DEVICES += cubietech_cubieboard2

define Device/cubietech_cubietruck
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Cubietech
  DEVICE_MODEL := Cubietruck
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-brcmfmac
  SOC := sun7i-a20
endef
TARGET_DEVICES += cubietech_cubietruck

define Device/friendlyarm_nanopi-m1-plus
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi M1 Plus
  DEVICE_PACKAGES:=kmod-leds-gpio kmod-brcmfmac \
	cypress-firmware-43430-sdio wpad-basic-mbedtls
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-m1-plus

define Device/friendlyarm_nanopi-neo
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-neo

define Device/friendlyarm_nanopi-neo-air
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO Air
  DEVICE_PACKAGES := kmod-leds-gpio kmod-brcmfmac \
	brcmfmac-firmware-43430a0-sdio wpad-basic-mbedtls
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-neo-air

define Device/friendlyarm_nanopi-r1
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R1
  DEVICE_PACKAGES := kmod-usb-net-rtl8152 kmod-leds-gpio \
	kmod-brcmfmac cypress-firmware-43430-sdio wpad-basic-mbedtls
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-r1

define Device/friendlyarm_zeropi
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := ZeroPi
  DEVICE_PACKAGES := kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_zeropi

define Device/lamobo_lamobo-r1
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Lamobo
  DEVICE_MODEL := Lamobo R1
  DEVICE_ALT0_VENDOR := Bananapi
  DEVICE_ALT0_MODEL := BPi-R1
  DEVICE_PACKAGES := kmod-ata-sunxi kmod-rtl8192cu wpad-basic-mbedtls
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Config cannot be migrated from swconfig to DSA
  SOC := sun7i-a20
endef
TARGET_DEVICES += lamobo_lamobo-r1

define Device/lemaker_bananapi
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi
  SOC := sun7i-a20
endef
TARGET_DEVICES += lemaker_bananapi

define Device/sinovoip_bananapi-m2-berry
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi M2 Berry
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-brcmfmac \
	cypress-firmware-43430-sdio wpad-basic-mbedtls
  SUPPORTED_DEVICES:=lemaker,bananapi-m2-berry
  SOC := sun8i-v40
endef
TARGET_DEVICES += sinovoip_bananapi-m2-berry

define Device/sinovoip_bananapi-m2-ultra
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi M2 Ultra
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-brcmfmac \
	brcmfmac-firmware-43430a0-sdio wpad-basic-mbedtls
  SUPPORTED_DEVICES:=lemaker,bananapi-m2-ultra
  SOC := sun8i-r40
endef
TARGET_DEVICES += sinovoip_bananapi-m2-ultra

define Device/lemaker_bananapro
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pro
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi kmod-brcmfmac \
	cypress-firmware-43362-sdio wpad-basic-mbedtls
  SOC := sun7i-a20
endef
TARGET_DEVICES += lemaker_bananapro

define Device/licheepi_licheepi-zero-dock
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LicheePi
  DEVICE_MODEL := Zero with Dock (V3s)
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-v3s
endef
TARGET_DEVICES += licheepi_licheepi-zero-dock

define Device/linksprite_pcduino3
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LinkSprite
  DEVICE_MODEL := pcDuino3
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-ata-sunxi kmod-rtl8xxxu \
	rtl8188eu-firmware
  SOC := sun7i-a20
endef
TARGET_DEVICES += linksprite_pcduino3

define Device/linksprite_pcduino3-nano
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := LinkSprite
  DEVICE_MODEL := pcDuino3 Nano
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi
  SOC := sun7i-a20
endef
TARGET_DEVICES += linksprite_pcduino3-nano

define Device/mele_m9
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Mele
  DEVICE_MODEL := M9
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtl8192cu
  SOC := sun6i-a31
endef
TARGET_DEVICES += mele_m9

define Device/merrii_hummingbird
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Merrii
  DEVICE_MODEL := Hummingbird
  DEVICE_PACKAGES:=kmod-brcmfmac cypress-firmware-43362-sdio wpad-basic-mbedtls
  SOC := sun6i-a31
endef
TARGET_DEVICES += merrii_hummingbird

define Device/olimex_a20-olinuxino-lime
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime

define Device/olimex_a20-olinuxino-lime2
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME2
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime2

define Device/olimex_a20-olinuxino-lime2-emmc
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME2
  DEVICE_VARIANT := eMMC
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime2-emmc

define Device/olimex_a20-olinuxino-micro
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-MICRO
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-micro

define Device/roofull_beelink-x2
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Roofull
  DEVICE_MODEL := Beelink-X2
  DEVICE_PACKAGES:=kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-brcmfmac cypress-firmware-43430-sdio wpad-basic-mbedtls
  SOC := sun8i-h3
endef
TARGET_DEVICES += roofull_beelink-x2

define Device/sinovoip_bananapi-m2-plus
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi M2+
  DEVICE_PACKAGES:=kmod-leds-gpio kmod-brcmfmac \
	brcmfmac-firmware-43430a0-sdio wpad-basic-mbedtls
  SOC := sun8i-h3
endef
TARGET_DEVICES += sinovoip_bananapi-m2-plus

define Device/sinovoip_bananapi-m3
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi M3
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-leds-gpio kmod-rtc-ac100 \
	kmod-brcmfmac cypress-firmware-43430-sdio wpad-basic-mbedtls
  SOC := sun8i-a83t
endef
TARGET_DEVICES += sinovoip_bananapi-m3

define Device/sinovoip_bananapi-p2-zero
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi P2 Zero
  DEVICE_PACKAGES:=kmod-leds-gpio kmod-brcmfmac \
	cypress-firmware-43430-sdio wpad-basic-mbedtls
  SOC := sun8i-h2-plus
endef
TARGET_DEVICES += sinovoip_bananapi-p2-zero

define Device/xunlong_orangepi-one
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi One
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-one

define Device/xunlong_orangepi-pc
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi PC
  DEVICE_PACKAGES:=kmod-gpio-button-hotplug
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-pc

define Device/xunlong_orangepi-pc-plus
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi PC Plus
  DEVICE_PACKAGES:=kmod-gpio-button-hotplug
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-pc-plus

define Device/xunlong_orangepi-plus
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-plus

define Device/xunlong_orangepi-r1
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1
  DEVICE_PACKAGES:=kmod-usb-net-rtl8152
  SOC := sun8i-h2-plus
endef
TARGET_DEVICES += xunlong_orangepi-r1

define Device/xunlong_orangepi-zero
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Zero
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h2-plus
endef
TARGET_DEVICES += xunlong_orangepi-zero

define Device/xunlong_orangepi-2
  $(call Device/FitImageGzip)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi 2
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-2
