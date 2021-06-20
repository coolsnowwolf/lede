# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

define Profile/olinuxino-maxi
  NAME:=Olimex OLinuXino Maxi/Mini boards
  PACKAGES += imx-bootlets uboot-mxs-mx23_olinuxino \
	  kmod-usb-net-smsc95xx kmod-gpio-mcp23s08 \
	  kmod-leds-gpio kmod-sound-core
endef

define Profile/olinuxino-maxi/Description
	Olimex OLinuXino Maxi/Mini boards
endef

$(eval $(call Profile,olinuxino-maxi))
