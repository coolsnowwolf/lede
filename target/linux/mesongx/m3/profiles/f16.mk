#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/f16
    NAME:= meson3 f16
    PACKAGES:= 
endef

f16_KERNELDTS:="meson3_f16"

define Profile/f16/Description
	Package set optimized for f16
endef

$(eval $(call Profile,f16))
