#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-none
  NAME:=Broadcom SoC, all Ethernet, No WiFi
  PACKAGES:=-wpad-basic-wolfssl kmod-b44 kmod-tg3 kmod-bgmac
endef

define Profile/Broadcom-none/Description
	Package set compatible with hardware any Broadcom BCM47xx or BCM535x
	SoC without any Wifi cards and b44, tg3 or bgmac Ethernet driver.
endef
$(eval $(call Profile,Broadcom-none))

