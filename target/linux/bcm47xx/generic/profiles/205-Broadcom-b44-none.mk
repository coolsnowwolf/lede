# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

define Profile/Broadcom-b44-none
  NAME:=Broadcom SoC, b44 Ethernet, No WiFi
  PACKAGES:=-wpad-basic kmod-b44
endef

define Profile/Broadcom-b44-none/Description
	Package set compatible with hardware older Broadcom BCM47xx or BCM535x
	SoC without any Wifi cards and b44 Ethernet driver.
endef
$(eval $(call Profile,Broadcom-b44-none))

