# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=x86
BOARDNAME:=x86
FEATURES:=squashfs vdi vmdk vhdx pcmcia fpu boot-part rootfs-part
SUBTARGETS:=64 generic geode legacy

KERNEL_PATCHVER:=6.6
KERNEL_TESTING_PATCHVER:=6.12

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += partx-utils mkf2fs e2fsprogs kmod-button-hotplug kmod-fs-f2fs kmod-mmc kmod-sdhci kmod-usb-hid amd64-microcode intel-microcode cfdisk \
kmod-atlantic kmod-bnx2x kmod-i40e kmod-i40evf kmod-iavf kmod-igb kmod-igbvf kmod-igc kmod-e1000e kmod-pcnet32 kmod-tulip kmod-via-velocity kmod-vmxnet3 \
kmod-sound-hda-core kmod-sound-hda-codec-realtek kmod-sound-hda-codec-via kmod-sound-via82xx kmod-sound-hda-intel kmod-sound-hda-codec-hdmi kmod-sound-i8x0 \
kmod-usb-net kmod-usb-net-asix-ax88179 kmod-usb-net-rtl8150 kmod-usb-net-aqc111 kmod-mlx4-core kmod-mlx5-core kmod-drm-i915 kmod-drm-amdgpu kmod-usb-audio \
kmod-usb-net-rtl8152-vendor kmod-r8125-rss kmod-r8126-rss kmod-r8168 kmod-8139cp kmod-8139too kmod-tg3 lm-sensors autocore-x86 automount autosamba ca-bundle

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_ISO_IMAGES),SetupHostCommand,Ignore),mkisofs, \
	Please install mkisofs. , \
	mkisofs -v 2>&1 , \
	genisoimage -v 2>&1 | grep genisoimage, \
	xorrisofs -v 2>&1 | grep xorriso \
))
