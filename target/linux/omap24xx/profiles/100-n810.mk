#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/n810-base
  NAME:=Nokia n810 (base)
  PACKAGES:= \
	block-mount \
	gpsd \
	ip iw kmod-p54-spi wpa-supplicant \
	dnsmasq dropbear \
	pwrtray-backend \
	kmod-fs-vfat kmod-fs-msdos \
	kmod-input-evdev \
	kmod-leds-gpio \
	kmod-usb-tahvo kmod-usb-gadget-eth \
	kmod-usb-net kmod-usb-net-cdc-ether kmod-usb-net-pegasus kmod-usb-net-rndis \
	kmod-usb-storage kmod-usb-hid \
	kmod-bluetooth kmod-bluetooth-hci-h4p bluez-utils bluez-hcidump \
	kmod-n810bm maemo-kexec calvaria \
	schedtool
endef

define Profile/n810-base/Description
	Minimal package set for Nokia n810 hardware.
endef
$(eval $(call Profile,n810-base))

