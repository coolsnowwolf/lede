#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/n200
    NAME:= meson8m2 n200
    PACKAGES:= 
endef

n200_KERNELDTS:="meson8m2_n200_2G"

define Profile/n200/Description
	Package set optimized for n200
endef

$(eval $(call Profile,n200))
