# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2010-2013 OpenWrt.org

define Profile/Broadcom-wl
  NAME:=Broadcom SoC, all Ethernet, BCM43xx WiFi (wl, proprietary)
  PACKAGES:=-wpad-basic kmod-b44 kmod-tg3 kmod-bgmac kmod-brcm-wl wlc nas
endef

define Profile/Broadcom-wl/Description
	Package set compatible with hardware any Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the proprietary Broadcom
	wireless "wl" driver and b44, tg3 or bgmac Ethernet driver.
endef

$(eval $(call Profile,Broadcom-wl))

