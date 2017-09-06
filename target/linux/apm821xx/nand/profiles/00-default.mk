#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
  PACKAGES := badblocks block-mount e2fsprogs \
	kmod-dm kmod-fs-ext4 kmod-fs-vfat kmod-usb-ledtrig-usbport \
	kmod-md-mod kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-iso8859-15 \
	kmod-nls-utf8 kmod-usb3 kmod-usb-dwc2 kmod-usb-storage \
	kmod-spi-gpio partx-utils
endef

define Profile/Default/Description
	Default package set
endef

$(eval $(call Profile,Default))
