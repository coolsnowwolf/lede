#
# Copyright (C) 2017 Hauke Mehrtens
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARDNAME:=Marvell Armada 3700LP (ARM64)
CPU_TYPE:=cortex-a53
FEATURES+=ext4

KERNELNAME:=Image dtbs
