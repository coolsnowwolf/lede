# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=ipq806x
BOARDNAME:=Qualcomm Atheros IPQ806X
FEATURES:=squashfs nand fpu ramdisk
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.10

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ata-ahci kmod-ata-ahci-platform \
	kmod-usb-ohci kmod-usb2 kmod-usb-ledtrig-usbport \
	kmod-phy-qcom-ipq806x-usb kmod-usb3 kmod-usb-dwc3-qcom \
	kmod-ath10k-ct wpad-openssl uboot-envtools \
	kmod-shortcut-fe kmod-qca-nss-drv kmod-qca-nss-ecm-standard kmod-qca-nss-gmac kmod-nss-ifb kmod-qca-nss-drv-pppoe \
	autocore-arm automount autosamba

$(eval $(call BuildTarget))
