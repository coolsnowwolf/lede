#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
  PRIORITY:=1
  NAME:=Generic (default)
endef

define Profile/Generic/Description
 Package set compatible with most Freescale i.MX 6 based boards.
endef

$(eval $(call Profile,Generic))
