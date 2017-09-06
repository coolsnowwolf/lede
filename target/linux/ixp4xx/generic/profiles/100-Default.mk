#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Default
  NAME:=Default Profile
  PACKAGES:=kmod-ath5k
endef

define Profile/Default/Description
	Default IXP4xx Profile
endef
$(eval $(call Profile,Default))

