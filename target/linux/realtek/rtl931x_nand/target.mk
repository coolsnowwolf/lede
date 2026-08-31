# SPDX-License-Identifier: GPL-2.0-only
ARCH:=mips
SUBTARGET:=rtl931x_nand
CPU_TYPE:=24kc
BOARD:=realtek
BOARDNAME:=Realtek MIPS RTL931X (NAND)
FEATURES+=nand

define Target/Description
	Build firmware images for Realtek RTL931x based boards.
endef
