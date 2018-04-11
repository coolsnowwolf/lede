#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
  NAME:=Broadcom SoC, BCM43xx WiFi (b43, brcmfmac, default)
  PACKAGES:=kmod-b43 kmod-brcmfmac
endef

define Profile/Generic/Description
	Package set compatible with any hardware using Broadcom BCM47xx or
	BCM535x SoCs with an ARM CPU like the BCM4707, BCM4708, BCM4709,
	BCM53010
endef

$(eval $(call Profile,Generic))

