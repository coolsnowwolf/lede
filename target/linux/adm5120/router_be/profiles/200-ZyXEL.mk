#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/P334WT
	NAME:=ZyXEL Prestige 334WT
	PACKAGES:=kmod-acx-mac80211
endef

define Profile/P334WT/Description
	Package set optimized for the ZyXEL Prestige 334WT board.
endef

define Profile/P335WT
	NAME:=ZyXEL Prestige 335WT
	PACKAGES:=kmod-acx-mac80211 kmod-usb-core kmod-usb-adm5120
endef

define Profile/P335WT/Description
	Package set optimized for the ZyXEL Prestige 335WT board.
endef

$(eval $(call Profile,P334WT))
$(eval $(call Profile,P335WT))
