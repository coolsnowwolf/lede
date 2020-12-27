BOARDNAME := MikroTik devices
FEATURES += minor nand squashfs

DEFAULT_PACKAGES += wpad-basic-wolfssl

define Target/Description
	Build firmware images for MikroTik devices based on Qualcomm Atheros
	MIPS SoCs (AR71xx, AR72xx, AR91xx, AR93xx, QCA95xx).
endef
