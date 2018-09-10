#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/EASY5120RT
	NAME:=Infineon EASY 5120-RT Reference Board
	PACKAGES:=kmod-usb-core kmod-usb-adm5120
endef

define Profile/EASY5120RT/Description
	Package set optimized for the Infineon EASY 5120-RT Reference Board
endef

$(eval $(call Profile,EASY5120RT))

define Profile/EASY5120PATA
	NAME:=Infineon EASY 5120P-ATA Reference Board
endef

define Profile/EASY5120RT/Description
	Package set optimized for the Infineon EASY 5120P-ATA Reference Board
endef

$(eval $(call Profile,EASY5120PATA))
