#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Generic
  NAME:=Generic profile
endef

define Profile/Generic/Description
   Default profile for Moschip MCS814x targets
endef
$(eval $(call Profile,Generic))

