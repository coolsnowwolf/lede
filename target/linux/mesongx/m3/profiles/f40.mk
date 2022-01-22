#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/f40
    NAME:= meson3 f40
    PACKAGES:= 
endef

f40_KERNELDTS:="meson3_f40"

define Profile/f40/Description
	Package set optimized for f40
endef

$(eval $(call Profile,f40))
