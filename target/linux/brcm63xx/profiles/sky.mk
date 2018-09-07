#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/SR102
  NAME:=SKY SR102
  PACKAGES:=kmod-b43 wpad-mini \
	kmod-usb2 kmod-usb-ohci
endef
define Profile/SR102/Description
  Package set optimized for SKY SR102.
endef
$(eval $(call Profile,SR102))
