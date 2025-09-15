# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 Toco Technologies <info@toco.ae>
#
include $(TOPDIR)/rules.mk

ARCH:=riscv64
BOARD:=sifiveu
BOARDNAME:=SiFive U-based RISC-V boards
FEATURES:=ext4
KERNELNAME:=Image dtbs
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.6
KERNEL_TESTING_PATCHVER:=6.12

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for the SiFive U-based boards
	(HiFive boards)
endef

$(eval $(call BuildTarget))
