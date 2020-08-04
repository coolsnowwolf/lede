#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
	NAME:=Default Profile
	PRIORITY:=1
	PACKAGES:=\
		kmod-i2c-gpio kmod-gpio-beeper kmod-hwmon-gpiofan \
		kmod-rtc-pcf8563 kmod-rtc-ds1307 kmod-usb3
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef

$(eval $(call Profile,Default))
