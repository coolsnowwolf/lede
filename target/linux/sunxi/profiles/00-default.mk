# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Yousong Zhou

define Profile/Default
  NAME:=Default Profile (all drivers)
  PACKAGES:= \
	kmod-ata-sunxi \
	kmod-brcmfmac \
	kmod-rtc-sunxi \
	kmod-rtl8192cu \
	kmod-rtl8xxxu \
	kmod-sun4i-emac \
	rtl8188eu-firmware \
	swconfig \
	wpad-basic-wolfssl
  PRIORITY := 1
endef

define Profile/Default/Description
  Default profile with package set compatible with most boards.
endef
$(eval $(call Profile,Default))
