#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/5GXI
	NAME:=Osbridge 5GXi/5XLi (Unofficial)
endef

define Profile/5GXI/Description
	Package set optimized for the OSBRiDGE 5GXi/5XLi boards.
endef

$(eval $(call Profile,5GXI))
