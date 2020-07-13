#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=malta
BOARDNAME:=MIPS Malta CoreLV board (qemu)
CPU_TYPE:=24kc
SUBTARGETS:=le be le64 be64
INITRAMFS_EXTRA_FILES:=
FEATURES:=cpiogz ext4 ramdisk squashfs targz

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += wpad-basic kmod-mac80211-hwsim kmod-pcnet32 mkf2fs e2fsprogs

$(eval $(call BuildTarget))
