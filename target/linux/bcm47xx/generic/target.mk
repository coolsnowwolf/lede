BOARDNAME:=Generic
FEATURES+=pcmcia

DEFAULT_PACKAGES += wpad-basic-wolfssl

define Target/Description
	Build generic firmware for all Broadcom BCM47xx and BCM53xx MIPS
	devices. It runs on both architectures BMIPS3300 and MIPS 74K.
endef
