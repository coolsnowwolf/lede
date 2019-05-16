#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-b44-wl
  NAME:=Broadcom SoC, b44 Ethernet, BCM43xx WiFi (wl, proprietary)
  PACKAGES:=-wpad-basic kmod-b44 kmod-brcm-wl wlc nas
endef

define Profile/Broadcom-b44-wl/Description
	Package set compatible with hardware older Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the proprietary Broadcom
	wireless "wl" driver and b44 Ethernet driver.
endef

$(eval $(call Profile,Broadcom-b44-wl))

