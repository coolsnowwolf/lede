# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2013 OpenWrt.org

define Profile/Generic
  NAME:=Octeon SoC
endef

define Profile/Generic/Description
   Base packages for Octeon boards.
endef

$(eval $(call Profile,Generic))
