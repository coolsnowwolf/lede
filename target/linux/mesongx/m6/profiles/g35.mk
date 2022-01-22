#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/g35
    NAME:= meson6 g35
    PACKAGES:= 
endef

g35_KERNELDTS:="meson6_g35"

define Profile/g35/Description
	Package set optimized for g35
endef

$(eval $(call Profile,g35))
