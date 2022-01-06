# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

define Profile/olinuxino-micro
  NAME:=Olimex OLinuXino Micro/Nano boards
  PACKAGES += imx-bootlets uboot-mxs-mx23_olinuxino \
	  kmod-gpio-mcp23s08 kmod-leds-gpio
endef

define Profile/olinuxino-micro/Description
	Olimex OLinuXino Micro/Nano boards
endef

$(eval $(call Profile,olinuxino-micro))
