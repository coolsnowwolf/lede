#
# Copyright (C) 2013-2016 OpenWrt.org
# Copyright (C) 2016 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
ifeq ($(SUBTARGET),cortexa8)

define Device/sun4i-a10-olinuxino-lime
  DEVICE_TITLE:=Olimex A10-OLinuXino-LIME
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SUPPORTED_DEVICES:=olimex,a10-olinuxino-lime
  SUNXI_DTS:=sun4i-a10-olinuxino-lime
endef

TARGET_DEVICES += sun4i-a10-olinuxino-lime


define Device/sun5i-a13-olimex-som
  DEVICE_TITLE:=Olimex A13 SOM
  DEVICE_PACKAGES:=kmod-rtl8192cu
  SUPPORTED_DEVICES:=olimex,a13-olinuxino
  SUNXI_DTS:=sun5i-a13-olinuxino
endef

TARGET_DEVICES += sun5i-a13-olimex-som


define Device/sun5i-a13-olinuxino
  DEVICE_TITLE:=Olimex A13-Olinuxino
  DEVICE_PACKAGES:=kmod-rtl8192cu
  SUPPORTED_DEVICES:=olimex,a13-olinuxino
  SUNXI_DTS:=sun5i-a13-olinuxino
endef

TARGET_DEVICES += sun5i-a13-olinuxino


define Device/sun4i-a10-cubieboard
  DEVICE_TITLE:=Cubietech Cubieboard
  DEVICE_PACKAGES:=kmod-ata-core kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi
  SUPPORTED_DEVICES:=cubietech,a10-cubieboard
  SUNXI_DTS:=sun4i-a10-cubieboard
endef

TARGET_DEVICES += sun4i-a10-cubieboard


define Device/sun4i-a10-pcduino
  DEVICE_TITLE:=LinkSprite pcDuino
  DEVICE_PACKAGES:=kmod-sun4i-emac kmod-rtc-sunxi kmod-rtl8192cu
  SUPPORTED_DEVICES:=linksprite,a10-pcduino
  SUNXI_DTS:=sun4i-a10-pcduino
endef

TARGET_DEVICES += sun4i-a10-pcduino


define Device/sun4i-a10-marsboard
  DEVICE_TITLE:=HAOYU Electronics Marsboard A10
  DEVICE_PACKAGES:=mod-ata-core kmod-ata-sunxi kmod-sun4i-emac kmod-rtc-sunxi sound-soc-sunxi
  SUPPORTED_DEVICES:=marsboard,a10-marsboard
  SUNXI_DTS:=sun4i-a10-marsboard
endef

TARGET_DEVICES += sun4i-a10-marsboard

endif
