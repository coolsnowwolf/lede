#
# Copyright (C) 2012 OpenWrt.org
#

SUBTARGET:=generic
BOARDNAME:=Texas Instruments AR7 boards

DEFAULT_PACKAGES+= kmod-mac80211 kmod-acx-mac80211 wpad-mini

define Target/Description
	Build firmware images for TI AR7 based routers.
endef
