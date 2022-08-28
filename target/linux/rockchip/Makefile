# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

BOARD:=rockchip
BOARDNAME:=Rockchip
FEATURES:=ext4 audio usb usbgadget display gpio fpu pci pcie rootfs-part boot-part squashfs
SUBTARGETS:=armv8

KERNEL_PATCHVER=5.15
KERNEL_TESTING_PATCHVER=5.19

define Target/Description
	Build firmware image for Rockchip SoC devices.
endef

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-envtools partx-utils e2fsprogs mkf2fs kmod-gpio-button-hotplug \
		    automount autocore-arm autosamba fdisk cfdisk e2fsprogs ethtool haveged htop \
		    luci-app-zerotier luci-app-ipsec-vpnd luci-app-diskman usbutils usb-modeswitch

KERNELNAME:=Image dtbs

$(eval $(call BuildTarget))
