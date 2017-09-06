#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Annex-A
  NAME:=Annex-A DSL firmware (default)
  PACKAGES:=kmod-pppoa ppp-mod-pppoa linux-atm atm-tools br2684ctl \
	    kmod-sangam-atm-annex-a
endef

define Profile/Annex-A/Description
	Package set compatible with Annex-A DSL lines (most countries).
endef
$(eval $(call Profile,Annex-A))

