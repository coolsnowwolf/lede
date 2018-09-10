#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/n810-gui
  $(call Profile/n810-base)
  NAME:=Nokia n810 (GUI)
  PACKAGES+= \
	xserver-xorg xinit xauth xkeyboard-config xkbdata xterm \
	xf86-video-omapfb xf86-input-tslib xf86-input-evdev \
	openbox tint2 matchbox-keyboard \
	pwrtray
endef

define Profile/n810-gui/Description
	Graphical user interface package set for Nokia n810 hardware.
endef
$(eval $(call Profile,n810-gui))

