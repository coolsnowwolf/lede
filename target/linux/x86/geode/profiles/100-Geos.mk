#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Geos
  NAME:=Geos
  PACKAGES:= \
		soloscli linux-atm br2684ctl ppp-mod-pppoa pppdump pppstats \
		hwclock flashrom tc kmod-pppoa kmod-8139cp kmod-mppe \
		kmod-usb-ohci-pci kmod-hwmon-lm90
endef

define Profile/Geos/Description
	Traverse Technologies Geos ADSL router
endef
$(eval $(call Profile,Geos))
