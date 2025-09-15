# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 LEDE project

define Profile/Default
  NAME:=Default Profile
  PACKAGES:=kmod-b43 wpad-basic-wolfssl
  PRIORITY:=1
endef

define Profile/Default/description
  Package set compatible with most boards.
endef

$(eval $(call Profile,Default))
