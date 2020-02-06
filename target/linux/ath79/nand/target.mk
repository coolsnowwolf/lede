BOARDNAME := Generic devices with NAND flash

FEATURES += squashfs nand

DEFAULT_PACKAGES += wpad

define Target/Description
	Firmware for boards using Qualcomm Atheros, MIPS-based SoCs
	in the ar72xx and subsequent series, with support for NAND flash
endef
