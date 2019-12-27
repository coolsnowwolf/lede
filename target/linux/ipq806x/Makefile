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
SUBTARGETS:=generic

KERNEL_PATCHVER:=4.19

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ata-core kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-usb-phy-qcom-dwc3 kmod-usb3 kmod-usb-dwc3-qcom \
	kmod-ath10k-ct wpad-basic \
	uboot-envtools

$(eval $(call BuildTarget))
