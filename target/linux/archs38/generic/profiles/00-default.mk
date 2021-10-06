# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 OpenWrt.org

define Profile/Default
	NAME:=Default Profile (all drivers)
	PACKAGES:= kmod-usb2 kmod-ath9k-htc wpad-basic-wolfssl
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,Default))
