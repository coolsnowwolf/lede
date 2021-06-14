#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
  NAME:=Generic
endef

define Profile/Generic/Description
	Generic Profile
endef
$(eval $(call Profile,Generic))
