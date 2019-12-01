#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/USR8200
  NAME:=USRobotics USR8200
  PACKAGES:=-wpad-basic kmod-scsi-core \
	kmod-usb-core kmod-usb-uhci kmod-usb2-pci kmod-usb-storage \
	kmod-fs-ext4 kmod-firewire kmod-firewire-ohci kmod-firewire-sbp2
endef

define Profile/USR8200/Description
	Package set optimized for the USRobotics USR8200
endef
$(eval $(call Profile,USR8200))

