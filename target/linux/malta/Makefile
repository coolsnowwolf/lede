# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2010-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

BOARD:=malta
BOARDNAME:=MIPS Malta CoreLV board (qemu)
SUBTARGETS:=le be le64 be64
INITRAMFS_EXTRA_FILES:=
FEATURES:=cpiogz ext4 ramdisk squashfs targz

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += wpad-basic-wolfssl kmod-mac80211-hwsim kmod-pcnet32 mkf2fs e2fsprogs

$(eval $(call BuildTarget))
