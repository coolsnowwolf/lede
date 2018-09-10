#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
	NAME:=Generic (default)
	PACKAGES:=
endef

define Profile/Generic/Description
	Generic package set compatible with most boards.
endef

define Profile/EB-214A
	NAME:=Generic EB-214A
	PACKAGES:=-wpad-mini -admswconfig -kmod-usb-adm5120 -kmod-ledtrig-adm5120-switch -dnsmasq kmod-usb-uhci kmod-usb2
endef

define Profile/EB-214A/Description
	Package set optimized for generic EB-214A boards.
endef

$(eval $(call Profile,Generic))
$(eval $(call Profile,EB-214A))

