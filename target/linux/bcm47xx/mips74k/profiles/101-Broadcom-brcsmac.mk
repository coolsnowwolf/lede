# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2014 OpenWrt.org

define Profile/Broadcom-mips74k-brcmsmac
  NAME:=Broadcom SoC, BCM43xx WiFi (brcmsmac)
  PACKAGES:=kmod-brcmsmac
endef

define Profile/Broadcom-mips74k-brcmsmac/Description
	Package set for devices with BCM43xx WiFi including mac80211 and
	brcmsmac driver.
endef

$(eval $(call Profile,Broadcom-mips74k-brcmsmac))

