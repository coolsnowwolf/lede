#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/NP27G
	NAME:=Compex NP27G
	PACKAGES:=kmod-ath5k kmod-usb-core kmod-usb-adm5120
endef

define Profile/NP27G/Description
	Package set optimized for the Compex NP27G.
endef

define Profile/NP28G
	NAME:=Compex NP28G
	PACKAGES:=kmod-ath5k kmod-usb-core kmod-usb-uhci kmod-usb2
endef

define Profile/NP28G/Description
	Package set optimized for the Compex NP28G.
endef

define Profile/WP54
	NAME:=Compex WP54 family
	PACKAGES:=kmod-ath5k
endef

define Profile/WP54/Description
	Package set optimized for the Compex WP54 family.
endef

$(eval $(call Profile,NP27G))
$(eval $(call Profile,NP28G))
$(eval $(call Profile,WP54))
