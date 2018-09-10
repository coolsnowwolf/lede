#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
	NAME:=Generic (default)
	PACKAGES:=
endef

define Profile/Generic/Description
	Generic package set compatible with most boards.
endef
$(eval $(call Profile,Generic))

