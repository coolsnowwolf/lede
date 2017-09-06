#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/olinuxino-micro
  NAME:=Olimex OLinuXino Micro/Nano boards
  PACKAGES += imx-bootlets uboot-mxs-mx23_olinuxino \
	  kmod-gpio-mcp23s08 kmod-leds-gpio kmod-ledtrig-heartbeat
endef

define Profile/olinuxino-micro/Description
	Olimex OLinuXino Micro/Nano boards
endef

$(eval $(call Profile,olinuxino-micro))
