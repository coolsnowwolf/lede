# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2023 Phytium Technology Co., Ltd.

ARCH:=aarch64
SUBTARGET:=phytiumpi
BOARDNAME:=PhytiumPi
FEATURES+=emmc nand display

DEFAULT_PACKAGES += hostapd hostapd-utils kmod-rtl8821cs kmod-cfg80211

define Target/Description
	Build firmware image for Phytium PhytiumPi devices.
endef
