# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 Toco Technologies <info@toco.ae>
#
include $(TOPDIR)/rules.mk

ARCH:=riscv64
BOARD:=d1
BOARDNAME:=AllWinner D1 RISC-V SoC
FEATURES:=ext4 squashfs
KERNELNAME:=Image dtbs

KERNEL_PATCHVER:=6.1

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for Allwinner D1 RISC-V boards
endef

$(eval $(call BuildTarget))
