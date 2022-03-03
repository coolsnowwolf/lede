# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021 Stijn Tintel <stijn@linux-ipv6.be>

include $(TOPDIR)/rules.mk

ARCH:=powerpc64
BOARD:=qoriq
BOARDNAME:=NXP QorIQ (PowerPC)
CPU_TYPE:=e5500
FEATURES:=boot-part ext4 fpu legacy-sdcard powerpc64 ramdisk root-part rtc source-only
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.10

KERNELNAME:=zImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += e2fsprogs mkf2fs uboot-envtools

$(eval $(call BuildTarget))
