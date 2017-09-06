#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Annex-B
  NAME:=Annex-B DSL firmware
  PACKAGES:=kmod-pppoa ppp-mod-pppoa linux-atm atm-tools br2684ctl \
	    kmod-sangam-atm-annex-b
endef

define Profile/Annex-B/Description
	Package set compatible with Annex-B DSL lines (Germany).
endef
$(eval $(call Profile,Annex-B))

