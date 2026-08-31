# SPDX-License-Identifier: GPL-2.0-only

ARCH:=mips
SUBTARGET:=en751627
BOARDNAME:=EN7516 and EN7527 based boards
CPU_TYPE:=24kc
KERNELNAME:=vmlinuz.bin

DEFAULT_PACKAGES += kmod-leds-gpio kmod-gpio-button-hotplug wpad-basic-mbedtls

define Target/Description
	Build firmware images for EcoNet EN7516 and EN7527 based boards.
endef
