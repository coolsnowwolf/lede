# SPDX-License-Identifier: GPL-2.0-only
ARCH:=mips
SUBTARGET:=rtl931x
CPU_TYPE:=24kc
BOARD:=realtek
BOARDNAME:=Realtek MIPS RTL931X

KERNEL_PATCHVER:=5.10

define Target/Description
	Build firmware images for Realtek RTL931x based boards.
endef
