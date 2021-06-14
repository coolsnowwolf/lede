# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
endef

define Profile/Default/Description
   Base packages for Octeon boards.
endef

$(eval $(call Profile,Default))
