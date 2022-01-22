#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=S905
BOARDNAME:=Amlogic S9 family
FEATURES+=audio boot-part ext4 fpu squashfs usbgadget
CPU_TYPE:=cortex-a53

define Target/Description
	Build firmware images for Amlogic S9 SoC devices.
endef

DEFAULT_PACKAGES += \
	e2fsprogs \
	kmod-sound-core \
	kmod-usb-hid \
	mkf2fs \
	partx-utils

KERNELNAME:=Image dtbs
