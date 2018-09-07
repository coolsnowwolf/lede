#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/dLAN_USB_Extender
  NAME:=Devolo dLAN USB Extender
  PACKAGES:=kmod-usb-serial kmod-usb-serial-ftdi
endef

define Profile/dLAN_USB_Extender/Description
	Package set specifically tuned for the Devolo dLAN USB Extender device.

	Binary packages are required to enable the power line portion of the
	device, these packages can be found in the following package feed:

	src-git dlan https://github.com/ffainelli/dlan-usb-extender-3rd-party.git

	Instructions are available here:
	http://ffainelli.github.io/dlan-usb-extender-3rd-party/
endef
$(eval $(call Profile,dLAN_USB_Extender))

