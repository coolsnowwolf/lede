#
# Copyright (C) 2013-2019 OpenWrt.org
# Copyright (C) 2016 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Device/cubietech_cubieboard2
  DEVICE_VENDOR := Cubietech
  DEVICE_MODEL := Cubieboard2
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SOC := sun7i-a20
endef
TARGET_DEVICES += cubietech_cubieboard2

define Device/cubietech_cubietruck
  DEVICE_VENDOR := Cubietech
  DEVICE_MODEL := Cubietruck
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-brcmfmac
  SOC := sun7i-a20
endef
TARGET_DEVICES += cubietech_cubietruck

define Device/friendlyarm_nanopi-m1-plus
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi M1 Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-leds-gpio kmod-ledtrig-heartbeat \
	kmod-brcmfmac brcmfmac-firmware-43430-sdio wpad-basic
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-m1-plus

define Device/friendlyarm_nanopi-neo
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-neo

define Device/friendlyarm_nanopi-neo-air
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi NEO Air
  DEVICE_PACKAGES := kmod-rtc-sunxi kmod-leds-gpio kmod-ledtrig-heartbeat \
	kmod-brcmfmac brcmfmac-firmware-43430-sdio wpad-basic
  SOC := sun8i-h3
endef
TARGET_DEVICES += friendlyarm_nanopi-neo-air

define Device/lamobo_lamobo-r1
  DEVICE_VENDOR := Lamobo
  DEVICE_MODEL := Lamobo R1
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtl8192cu swconfig wpad-basic
  SOC := sun7i-a20
endef
TARGET_DEVICES += lamobo_lamobo-r1

define Device/lemaker_bananapi
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi
  SOC := sun7i-a20
endef
TARGET_DEVICES += lemaker_bananapi

define Device/lemaker_bananapi-m2-ultra
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pi M2 Ultra
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi
  SOC := sun8i-r40
endef
TARGET_DEVICES += lemaker_bananapi-m2-ultra

define Device/lemaker_bananapro
  DEVICE_VENDOR := LeMaker
  DEVICE_MODEL := Banana Pro
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-sunxi kmod-brcmfmac
  SOC := sun7i-a20
endef
TARGET_DEVICES += lemaker_bananapro

define Device/linksprite_pcduino3
  DEVICE_VENDOR := LinkSprite
  DEVICE_MODEL := pcDuino3
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-ata-sunxi kmod-rtl8xxxu \
	rtl8188eu-firmware
  SOC := sun7i-a20
endef
TARGET_DEVICES += linksprite_pcduino3

define Device/mele_m9
  DEVICE_VENDOR := Mele
  DEVICE_MODEL := M9
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-rtl8192cu
  SOC := sun6i-a31
endef
TARGET_DEVICES += mele_m9

define Device/olimex_a20-olinuxino-lime
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime

define Device/olimex_a20-olinuxino-lime2
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME2
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime2

define Device/olimex_a20-olinuxino-lime2-emmc
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-LIME2
  DEVICE_VARIANT := eMMC
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-lime2-emmc

define Device/olimex_a20-olinuxino-micro
  DEVICE_VENDOR := Olimex
  DEVICE_MODEL := A20-OLinuXino-MICRO
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SOC := sun7i
endef
TARGET_DEVICES += olimex_a20-olinuxino-micro

define Device/sinovoip_bananapi-m2-plus
  DEVICE_VENDOR := Sinovoip
  DEVICE_MODEL := Banana Pi M2+
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-leds-gpio kmod-ledtrig-heartbeat \
	kmod-brcmfmac brcmfmac-firmware-43430a0-sdio wpad-basic
  SOC := sun8i-h3
endef
TARGET_DEVICES += sinovoip_bananapi-m2-plus

define Device/xunlong_orangepi-one
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi One
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-one

define Device/xunlong_orangepi-pc
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi PC
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-gpio-button-hotplug
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-pc

define Device/xunlong_orangepi-pc-plus
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi PC Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-gpio-button-hotplug
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-pc-plus

define Device/xunlong_orangepi-plus
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-plus

define Device/xunlong_orangepi-r1
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-usb-net kmod-usb-net-rtl8152
  SOC := sun8i-h2-plus
endef
TARGET_DEVICES += xunlong_orangepi-r1

define Device/xunlong_orangepi-zero
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi Zero
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h2-plus
endef
TARGET_DEVICES += xunlong_orangepi-zero

define Device/xunlong_orangepi-2
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi 2
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SOC := sun8i-h3
endef
TARGET_DEVICES += xunlong_orangepi-2
