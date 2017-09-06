#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/None
  NAME:=No WiFi
  PACKAGES:=
endef

define Profile/None/Description
	Package set without WiFi support
endef
$(eval $(call Profile,None))

