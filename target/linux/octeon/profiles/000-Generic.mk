#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
endef

define Profile/Default/Description
   Base packages for Octeon boards.
endef

$(eval $(call Profile,Default))
