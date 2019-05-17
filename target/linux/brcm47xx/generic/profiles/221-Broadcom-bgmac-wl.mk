#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-bgmac-wl
  NAME:=Broadcom SoC, bgmac Ethernet, BCM43xx WiFi (wl, proprietary)
  PACKAGES:=-wpad-basic kmod-bgmac kmod-brcm-wl wlc nas
endef

define Profile/Broadcom-bgmac-wl/Description
	Package set compatible with hardware newer Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the proprietary Broadcom
	wireless "wl" driver and bgmac Ethernet driver.
endef

$(eval $(call Profile,Broadcom-bgmac-wl))

