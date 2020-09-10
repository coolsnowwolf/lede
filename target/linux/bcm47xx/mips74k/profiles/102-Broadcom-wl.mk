#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-mips74k-wl
  NAME:=Broadcom SoC, BCM43xx WiFi (proprietary wl)
  PACKAGES:=-wpad-basic-wolfssl kmod-brcm-wl wlc nas
endef

define Profile/Broadcom-mips74k-wl/Description
	Package set for devices with BCM43xx WiFi including proprietary (and
	closed source) driver "wl".
endef

$(eval $(call Profile,Broadcom-mips74k-wl))

