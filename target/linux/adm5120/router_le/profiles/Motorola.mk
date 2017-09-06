#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/PMUGW
	NAME:=Motorola Powerline MU Gateway (EXPERIMENTAL)
endef

define Profile/PMUGW/Description
	Package set optimized for the Motorola Powerline MU Gateway board
endef

$(eval $(call Profile,PMUGW))
