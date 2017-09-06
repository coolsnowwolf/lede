#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/FSG3
  NAME:=Freecom FSG-3
  PACKAGES:= \
	kmod-ath5k \
	kmod-usb-core kmod-usb-uhci kmod-usb2-pci kmod-usb-storage \
	kmod-fs-ext4 kmod-fs-reiserfs
endef

define Profile/FSG3/Description
	Package set optimized for the Freecom FSG-3
endef
$(eval $(call Profile,FSG3))

