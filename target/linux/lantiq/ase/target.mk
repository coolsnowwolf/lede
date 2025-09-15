# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2016 LEDE Project

ARCH:=mips
SUBTARGET:=ase
BOARDNAME:=Amazon-SE
FEATURES+=atm mips16 small_flash
CPU_TYPE:=mips32

DEFAULT_PACKAGES+=kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe

define Target/Description
	Lantiq Amazon-SE Boards
endef
