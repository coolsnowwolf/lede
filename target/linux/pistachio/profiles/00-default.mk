# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 OpenWrt.org

define Profile/Default
  NAME:=Default Profile
  PRIORITY:=1
  PACKAGES:=
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
