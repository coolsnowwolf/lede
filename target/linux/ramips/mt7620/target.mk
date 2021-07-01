#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=mt7620
BOARDNAME:=MT7620 based boards
FEATURES+=usb nand ramdisk
CPU_TYPE:=24kc

DEFAULT_PACKAGES += -ddns-scripts_aliyun -ddns-scripts_dnspod kmod-rt2800-soc wpad-openssl swconfig

define Target/Description
	Build firmware images for Ralink MT7620 based boards.
endef

