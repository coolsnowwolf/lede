#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define AddDepends/nls
  DEPENDS+= +kmod-nls-base $(foreach cp,$(1),+kmod-nls-$(cp))
endef

define AddDepends/rfkill
  DEPENDS+= +USE_RFKILL:kmod-rfkill $(1)
endef
