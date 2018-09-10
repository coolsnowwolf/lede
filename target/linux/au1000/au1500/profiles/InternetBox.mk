#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/InternetBox
  NAME:=T-Mobile "InternetBox"
  PACKAGES:=kmod-ath5k kmod-usb-core kmod-usb-ohci kmod-usb-serial kmod-usb-serial-option \
		comgt
endef

define Profile/InternetBox/Description
	Package set for the T-Mobile "InternetBox" (TMD SB1-S)
endef

$(eval $(call Profile,InternetBox))
