# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 Weijie Gao <hackpascal@gmail.com>

include $(TOPDIR)/rules.mk

ARCH:=loongarch64
BOARD:=loongarch64
BOARDNAME:=Loongson LoongArch
FEATURES:=audio display squashfs ext4 pcie boot-part rootfs-part rtc usb targz

KERNEL_PATCHVER:=6.6

KERNELNAME:=vmlinuz.efi

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	partx-utils blkid e2fsprogs grub2-efi-loongarch64 htop wpad-openssl

$(eval $(call BuildTarget))
