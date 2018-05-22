#
# Copyright (C) 2018 Sartura Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARDNAME:=Marvell Armada 7k/8k (ARM64)
CPU_TYPE:=cortex-a72
FEATURES+=ext4

KERNELNAME:=Image dtbs
