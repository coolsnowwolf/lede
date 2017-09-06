#
# Copyright (C) 2007,2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/BR6104K
	NAME:=Edimax BR-6104K (Unofficial)
	PACKAGES:=-wpad-mini
endef

define Profile/BR6104K/Description
	Package set optimized for the Edimax BR-6104K
endef

define Profile/BR6104KP
	NAME:=Edimax BR-6104KP (Unofficial)
	PACKAGES:=-wpad-mini kmod-usb-core kmod-usb-adm5120
endef

define Profile/BR6104KP/Description
	Package set optimized for the Edimax BR-6104KP
endef

define Profile/BR6104WG
	NAME:=Edimax BR-6104Wg (Unofficial, No WiFi)
	PACKAGES:=-wpad-mini
endef

define Profile/BR6104WG/Description
	Package set optimized for the Edimax BR-6104Wg
endef

define Profile/BR6114WG
	NAME:=Edimax BR-6114WG (Unofficial, No WiFi)
	PACKAGES:=-wpad-mini
endef

define Profile/BR6114WG/Description
	Package set optimized for the Edimax BR-6114WG
endef

$(eval $(call Profile,BR6104K))
$(eval $(call Profile,BR6104KP))
$(eval $(call Profile,BR6104WG))
$(eval $(call Profile,BR6114WG))
