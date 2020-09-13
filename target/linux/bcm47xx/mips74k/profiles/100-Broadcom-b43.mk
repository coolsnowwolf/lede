#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-mips74k-b43
  NAME:=Broadcom SoC, BCM43xx WiFi (b43)
  PACKAGES:=kmod-b43
endef

define Profile/Broadcom-mips74k-b43/Description
	Package set for devices with BCM43xx WiFi including mac80211 and b43
	driver.
endef

$(eval $(call Profile,Broadcom-mips74k-b43))

