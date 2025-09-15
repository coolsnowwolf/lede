# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2007-2013 OpenWrt.org

define Profile/Broadcom-bgmac-b43
  NAME:=Broadcom SoC, bgmac Ethernet, BCM43xx WiFi (b43)
  PACKAGES:=kmod-bgmac kmod-b43
endef

define Profile/Broadcom-bgmac-b43/Description
	Package set compatible with hardware newer Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the mac80211 and b43
	drivers and bgmac Ethernet driver.
endef

$(eval $(call Profile,Broadcom-bgmac-b43))

