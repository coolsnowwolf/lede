#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/RouterBoard
	NAME:=Mikrotik RouterBoard 1xx family
	PACKAGES:=kmod-ath5k kmod-pata-rb153-cf
endef

define Profile/RouterBoard/Description
	Package set compatible with the RouterBoard RB1xx devices. Contains RouterOS to OpenWrt\\\
	installation scripts.
endef

$(eval $(call Profile,RouterBoard))
