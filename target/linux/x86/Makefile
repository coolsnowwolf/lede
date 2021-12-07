# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=x86
BOARDNAME:=x86
FEATURES:=squashfs vdi vmdk pcmcia fpu boot-part rootfs-part
SUBTARGETS:=64 generic legacy geode 

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += partx-utils mkf2fs e2fsprogs kmod-button-hotplug wpad kmod-usb-hid kmod-mmc-spi kmod-sdhci usbutils pciutils \
kmod-ath5k kmod-ath9k kmod-ath9k-htc kmod-ath10k kmod-rt2800-usb kmod-alx kmod-e1000e kmod-igb kmod-igc kmod-igbvf kmod-ixgbe kmod-ixgbevf kmod-iavf kmod-bnx2x kmod-pcnet32 kmod-tulip kmod-via-velocity kmod-vmxnet3 kmod-i40e kmod-i40evf kmod-r8125 kmod-8139cp kmod-8139too kmod-fs-f2fs cfdisk \
htop lm-sensors autocore-x86 automount autosamba luci-app-adbyby-plus luci-app-ipsec-vpnd luci-proto-bonding \
luci-app-unblockmusic luci-app-zerotier luci-app-xlnetacc ddns-scripts_aliyun ddns-scripts_dnspod ca-bundle luci-app-wireguard luci-app-qbittorrent luci-app-ttyd \
ath10k-firmware-qca988x ath10k-firmware-qca9888 ath10k-firmware-qca9984 brcmfmac-firmware-43602a1-pcie \
kmod-sound-hda-core kmod-sound-hda-codec-realtek kmod-sound-hda-codec-via kmod-sound-via82xx kmod-sound-hda-intel kmod-sound-hda-codec-hdmi kmod-sound-i8x0 kmod-usb-audio \
kmod-usb-net kmod-usb-net-asix kmod-usb-net-asix-ax88179 kmod-usb-net-rtl8150 kmod-usb-net-rtl8152-vendor kmod-mlx4-core kmod-mlx5-core

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_ISO_IMAGES),SetupHostCommand,Ignore),mkisofs, \
	Please install mkisofs. , \
	mkisofs -v 2>&1 , \
	genisoimage -v 2>&1 | grep genisoimage, \
	xorrisofs -v 2>&1 | grep xorriso \
))
