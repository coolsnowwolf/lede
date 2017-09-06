#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-b44-none
  NAME:=Broadcom SoC, b44 Ethernet, No WiFi
  PACKAGES:=-wpad-mini kmod-b44
endef

define Profile/Broadcom-b44-none/Description
	Package set compatible with hardware older Broadcom BCM47xx or BCM535x
	SoC without any Wifi cards and b44 Ethernet driver.
endef
$(eval $(call Profile,Broadcom-b44-none))

