# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2011 OpenWrt.org

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
  PACKAGES := kmod-usb-dwc2 kmod-usb-ledtrig-usbport kmod-usb-storage kmod-fs-vfat wpad-basic
endef

define Profile/Default/Description
	Default package set
endef

$(eval $(call Profile,Default))
