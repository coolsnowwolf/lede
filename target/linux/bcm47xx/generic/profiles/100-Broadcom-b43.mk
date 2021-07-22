# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2007-2013 OpenWrt.org

define Profile/Broadcom-b43
  NAME:=Broadcom SoC, all Ethernet, BCM43xx WiFi (b43, default)
  PACKAGES:=kmod-b44 kmod-tg3 kmod-bgmac kmod-b43 kmod-b43legacy
endef

define Profile/Broadcom-b43/Description
	Package set compatible with hardware any Broadcom BCM47xx or BCM535x
	SoC with Broadcom BCM43xx Wifi cards using the mac80211, b43 and
	b43legacy drivers and b44, tg3 or bgmac Ethernet driver.
endef

$(eval $(call Profile,Broadcom-b43))

