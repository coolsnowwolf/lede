# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2014 OpenWrt.org

define Profile/Broadcom-mips74k-b43
  NAME:=Broadcom SoC, BCM43xx WiFi (b43)
  PACKAGES:=kmod-b43
endef

define Profile/Broadcom-mips74k-b43/Description
	Package set for devices with BCM43xx WiFi including mac80211 and b43
	driver.
endef

$(eval $(call Profile,Broadcom-mips74k-b43))

