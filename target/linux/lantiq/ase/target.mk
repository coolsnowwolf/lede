#
# Copyright (C) 2016 LEDE Project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
ARCH:=mips
SUBTARGET:=ase
BOARDNAME:=Amazon-SE
FEATURES:=squashfs atm mips16 small_flash
CPU_TYPE:=mips32

DEFAULT_PACKAGES+=kmod-leds-gpio kmod-gpio-button-hotplug \
	kmod-ltq-adsl-ase kmod-ltq-adsl-ase-mei \
	kmod-ltq-adsl-ase-fw-b kmod-ltq-atm-ase \
	ltq-adsl-app ppp-mod-pppoe

define Target/Description
	Lantiq Amazon-SE Boards
endef
