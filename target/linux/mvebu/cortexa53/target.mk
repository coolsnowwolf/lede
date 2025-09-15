# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2017 Hauke Mehrtens

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARDNAME:=Marvell Armada 3700LP (ARM64)
CPU_TYPE:=cortex-a53
FEATURES+=ext4
DEFAULT_PACKAGES+=e2fsprogs ethtool mkf2fs partx-utils

KERNELNAME:=Image dtbs
