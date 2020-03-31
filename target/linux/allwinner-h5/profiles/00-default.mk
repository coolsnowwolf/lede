#
# Copyright (C) 2017 Yousong Zhou
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
  NAME:=FriendlyELEC NanoPi/NanoPC
  PACKAGES:=
  PRIORITY := 1
endef

define Profile/Default/Description
  Default profile with package set compatible with most boards.
endef
$(eval $(call Profile,Default))
