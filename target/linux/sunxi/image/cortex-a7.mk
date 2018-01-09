#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2016 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(SUBTARGET),cortexa7)

define Device/sun7i-a20-olinuxino-lime
  DEVICE_TITLE:=Olimex A20-OLinuXino-LIME
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-rtc-sunxi
  SUPPORTED_DEVICES:=olimex,a20-olinuxino-lime
  SUNXI_DTS:=sun7i-a20-olinuxino-lime
endef

TARGET_DEVICES += sun7i-a20-olinuxino-lime


define Device/sun7i-a20-olinuxino-lime2
  DEVICE_TITLE:=Olimex A20-OLinuXino-LIME2
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SUPPORTED_DEVICES:=olimex,a20-olinuxino-lime2
  SUNXI_DTS:=sun7i-a20-olinuxino-lime2
endef

TARGET_DEVICES += sun7i-a20-olinuxino-lime2


define Device/sun7i-a20-olinuxino-lime2-emmc
  DEVICE_TITLE:=Olimex A20-OLinuXino-LIME2-eMMC
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-rtc-sunxi kmod-usb-hid
  SUPPORTED_DEVICES:=olimex,a20-olinuxino-lime2-emmc
  SUNXI_DTS:=sun7i-a20-olinuxino-lime2-emmc
endef

TARGET_DEVICES += sun7i-a20-olinuxino-lime2-emmc


define Device/sun7i-a20-olinuxino-micro
  DEVICE_TITLE:=Olimex A20-Olinuxino Micro
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SUPPORTED_DEVICES:=olimex,a20-olinuxino-micro
  SUNXI_DTS:=sun7i-a20-olinuxino-micro
endef

TARGET_DEVICES += sun7i-a20-olinuxino-micro


define Device/sun7i-a20-bananapi
  DEVICE_TITLE:=LeMaker Banana Pi
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-core kmod-ata-sunxi
  SUPPORTED_DEVICES:=lemaker,bananapi
  SUNXI_DTS:=sun7i-a20-bananapi
endef

TARGET_DEVICES += sun7i-a20-bananapi


define Device/sun7i-a20-bananapro
  DEVICE_TITLE:=LeMaker Banana Pro
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-ata-core kmod-ata-sunxi kmod-brcmfmac
  SUPPORTED_DEVICES:=lemaker,bananapro
  SUNXI_DTS:=sun7i-a20-bananapro
endef

TARGET_DEVICES += sun7i-a20-bananapro


define Device/sun7i-a20-cubieboard2
  DEVICE_TITLE:=Cubietech Cubieboard2
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SUPPORTED_DEVICES:=cubietech,cubieboard2
  SUNXI_DTS:=sun7i-a20-cubieboard2
endef

TARGET_DEVICES += sun7i-a20-cubieboard2


define Device/sun7i-a20-cubietruck
  DEVICE_TITLE:=Cubietech Cubietruck
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-rtc-sunxi kmod-brcmfmac
  SUPPORTED_DEVICES:=cubietech,cubietruck
  SUNXI_DTS:=sun7i-a20-cubietruck
endef

TARGET_DEVICES += sun7i-a20-cubietruck


define Device/sun7i-a20-lamobo-r1
  DEVICE_TITLE:=Lamobo R1
  DEVICE_PACKAGES:=kmod-ata-sunxi kmod-rtl8192cu swconfig wpad-mini
  SUPPORTED_DEVICES:=lamobo,lamobo-r1
  SUNXI_DTS:=sun7i-a20-lamobo-r1
endef

TARGET_DEVICES += sun7i-a20-lamobo-r1


define Device/sun6i-a31-m9
  DEVICE_TITLE:=Mele M9 top set box
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-rtl8192cu
  SUPPORTED_DEVICES:=mele,m9
  SUNXI_DTS:=sun6i-a31-m9
endef

TARGET_DEVICES += sun6i-a31-m9


define Device/sun8i-h2-plus-orangepi-r1
  DEVICE_TITLE:=Xunlong Orange Pi R1
  DEVICE_PACKAGES:=kmod-rtc-sunxi kmod-usb-net kmod-usb-net-rtl8152
  SUPPORTED_DEVICES:=xunlong,orangepi-r1
  SUNXI_DTS:=sun8i-h2-plus-orangepi-r1
endef

TARGET_DEVICES += sun8i-h2-plus-orangepi-r1


define Device/sun8i-h3-nanopi-m1-plus
  DEVICE_TITLE:=FriendlyArm NanoPi M1 Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi \
	kmod-leds-gpio kmod-ledtrig-heartbeat \
	kmod-brcmfmac brcmfmac-firmware-43430-sdio wpad-mini
  SUPPORTED_DEVICES:=friendlyarm,nanopi-m1-plus
  SUNXI_DTS:=sun8i-h3-nanopi-m1-plus
endef

TARGET_DEVICES += sun8i-h3-nanopi-m1-plus


define Device/sun8i-h3-nanopi-neo
  DEVICE_TITLE:=FriendlyARM NanoPi NEO
  SUPPORTED_DEVICES:=friendlyarm,nanopi-neo
  SUNXI_DTS:=sun8i-h3-nanopi-neo
endef

TARGET_DEVICES += sun8i-h3-nanopi-neo


define Device/sun8i-h3-orangepi-plus
  DEVICE_TITLE:=Xunlong Orange Pi Plus
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SUPPORTED_DEVICES:=xunlong,orangepi-plus
  SUNXI_DTS:=sun8i-h3-orangepi-plus
endef

TARGET_DEVICES += sun8i-h3-orangepi-plus

define Device/sun8i-h3-orangepi-2
  DEVICE_TITLE:=Xunlong Orange Pi 2
  DEVICE_PACKAGES:=kmod-rtc-sunxi
  SUPPORTED_DEVICES:=xunlong,orangepi-2
  SUNXI_DTS:=sun8i-h3-orangepi-2
endef

TARGET_DEVICES += sun8i-h3-orangepi-2


define Device/sun7i-a20-pcduino3
  DEVICE_TITLE:=LinkSprite pcDuino3
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-ata-core kmod-ata-sunxi kmod-rtl8xxxu rtl8188eu-firmware
  SUPPORTED_DEVICES:=linksprite,pcduino3
  SUNXI_DTS:=sun7i-a20-pcduino3
endef

TARGET_DEVICES += sun7i-a20-pcduino3

endif
