# SPDX-License-Identifier: GPL-2.0-only

ARCH:=mipsel
SUBTARGET:=en7528
BOARDNAME:=EN7528 based boards
CPU_TYPE:=24kc
KERNELNAME:=vmlinuz.bin

DEFAULT_PACKAGES += kmod-leds-gpio kmod-gpio-button-hotplug wpad-basic-mbedtls

define Target/Description
	Build firmware images for EcoNet EN7528 based boards.
endef
