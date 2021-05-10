# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2018 Sartura Ltd.

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARDNAME:=Marvell Armada 7k/8k (ARM64)
CPU_TYPE:=cortex-a72
FEATURES+=ext4
DEFAULT_PACKAGES+=e2fsprogs ethtool mkf2fs partx-utils

KERNELNAME:=Image dtbs
