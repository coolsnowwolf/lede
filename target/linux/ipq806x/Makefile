# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=ipq806x
BOARDNAME:=Qualcomm Atheros IPQ806X
FEATURES:=squashfs nand fpu ramdisk
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=4.14

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk

# TODO: 4.19 drops kmod-usb-dwc3-of-simple which is replaced by_kmod-usb-dwc3-qcom
# 4.19 also integrates kmod-usb-phy-qcom-dwc3
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ata-core kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-usb3 kmod-usb-dwc3-of-simple kmod-usb-phy-qcom-dwc3 \
	kmod-ath10k-ct wpad uboot-envtools e2fsprogs \
	automount autosamba v2ray shadowsocks-libev-ss-redir shadowsocksr-libev-server \
	luci-app-ipsec-vpnd luci-app-unblockmusic luci-app-zerotier ca-certificates

$(eval $(call BuildTarget))
