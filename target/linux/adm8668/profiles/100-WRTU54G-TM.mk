#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/WRTU54G-TM
  NAME:=Linksys WRTU54G T-Mobile (Default)
  PACKAGES:=kmod-rt61-pci kmod-usb2 wpad-basic
endef

define Profile/WRTU54G-TM/Description
	Package set compatible with the Linksys WRTU54G T-Mobile
endef
$(eval $(call Profile,WRTU54G-TM))

