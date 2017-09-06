#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
  NAME:=Generic
  PACKAGES:= \
		soloscli linux-atm br2684ctl ppp-mod-pppoa pppdump pppstats \
		hwclock flashrom tc kmod-pppoa kmod-8139cp kmod-mppe \
		kmod-usb-ohci-pci kmod-hwmon-lm90 \
		kmod-via-rhine
endef

define Profile/Generic/Description
	Generic Profile for all Geode boards.
endef
$(eval $(call Profile,Generic))
