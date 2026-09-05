# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2026

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=qcom
BOARDNAME:=Qualcomm UEFI ARM64 Boards
FEATURES:=audio display ext4 fpu gpio pci pcie squashfs boot-part rootfs-part rtc usb targz cpiogz
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.18
KERNELNAME:=vmlinuz.efi dtbs

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	partx-utils blkid e2fsprogs grub2-efi-arm \
	kmod-usb-net-rtl8152 kmod-video-core \
	kmod-fs-ext4 kmod-fs-f2fs losetup mkf2fs f2fsck \
	kmod-fs-vfat autocore-arm

$(eval $(call BuildTarget))
