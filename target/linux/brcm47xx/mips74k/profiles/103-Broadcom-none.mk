#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Broadcom-mips74k-none
  NAME:=Broadcom SoC, No WiFi
  PACKAGES:=-wpad-basic
endef

define Profile/Broadcom-mips74k-none/Description
	Package set for devices without a WiFi.
endef

$(eval $(call Profile,Broadcom-mips74k-none))

