#
# Copyright (C) 2018 Jianhui Zhao <jianhuizhao329@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

SUBTARGET:=s5pv210
BOARDNAME:=s5pv210 based boards
FEATURES+=fpu
CPU_TYPE:=cortex-a8
CPU_SUBTYPE:=neon

define Target/Description
	Build firmware images for Samsung s5pv210 based boards.
endef

