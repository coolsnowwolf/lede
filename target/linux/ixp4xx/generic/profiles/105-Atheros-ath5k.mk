#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Atheros-ath5k
  NAME:=Atheros WiFi (atk5k)
  PACKAGES:=kmod-ath5k
endef

define Profile/Atheros-ath5k/Description
	Package set compatible with hardware using Atheros WiFi cards
endef
$(eval $(call Profile,Atheros-ath5k))

