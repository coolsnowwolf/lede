#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7620nand
BOARDNAME:=MT7620 based boards with NAND flash
FEATURES+=nand usb
CPU_TYPE:=24kc

DEFAULT_PACKAGES += kmod-rt2800-pci kmod-rt2800-soc wpad-mini

define Target/Description
	Build firmware images for Ralink MT7620 based boards with NAND flash.
endef

