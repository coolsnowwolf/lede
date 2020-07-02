#
# Copyright (C) 2007-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-bgmac-brcmsmac
  NAME:=Broadcom SoC, bgmac Ethernet, BCM43xx WiFi (brcmsmac)
  PACKAGES:=kmod-bgmac kmod-brcmsmac
endef

define Profile/Broadcom-bgmac-brcmsmac/Description
	Package set compatable with newer gigabit + N based bcm47xx SoCs with
	Broadcom BCM43xx Wifi cards using the mac80211 brcmsmac driver and
	bgmac Ethernet driver.
endef

$(eval $(call Profile,Broadcom-bgmac-brcmsmac))

