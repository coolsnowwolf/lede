# SPDX-License-Identifier: GPL-2.0-only
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=siflower
BOARDNAME:=Siflower SoCs
FEATURES:=squashfs usb usbgadget source-only
SUBTARGETS:=sf19a2890

KERNEL_PATCHVER:=6.6

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug \
	uboot-envtools

$(eval $(call BuildTarget))
