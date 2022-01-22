#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/g18
    NAME:= meson6 g18
    PACKAGES:= 
endef

g18_KERNELDTS:="meson6_g18"

define Profile/g18/Description
	Package set optimized for g18
endef

$(eval $(call Profile,g18))
