# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2013 OpenWrt.org

define Profile/Broadcom-bgmac-none
  NAME:=Broadcom SoC, bgmac Ethernet, No WiFi
  PACKAGES:=-wpad-basic-wolfssl kmod-bgmac
endef

define Profile/Broadcom-bgmac-none/Description
	Package set compatible with hardware newer Broadcom BCM47xx or BCM535x
	SoC without any Wifi cards and bgmac Ethernet driver.
endef
$(eval $(call Profile,Broadcom-bgmac-none))

