# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=aarch64
BOARD:=bcm4908
BOARDNAME:=Broadcom BCM4908 (ARMv8A CPUs Brahma-B53)
FEATURES:=squashfs nand usb gpio
CPU_TYPE:=cortex-a53
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.10

define Target/Description
	Build firmware images for Broadcom BCM4908 SoC family routers.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=Image dtbs

DEFAULT_PACKAGES += \
	bcm4908img fdt-utils uboot-envtools \
	kmod-gpio-button-hotplug \
	kmod-usb-ohci kmod-usb2 kmod-usb3

$(eval $(call BuildTarget))
