#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
	NAME:=Default Profile (all drivers)
	PACKAGES:= kmod-mwlwifi wpad-mini swconfig mkf2fs e2fsprogs \
		   mwlwifi-firmware-88w8864 mwlwifi-firmware-88w8964
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef

$(eval $(call Profile,Default))
