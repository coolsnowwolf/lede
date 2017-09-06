BOARDNAME:=Mikrotik devices with NAND/NOR flash
FEATURES += squashfs ramdisk minor nand

DEFAULT_PACKAGES += nand-utils

define Target/Description
	Build firmware images for Atheros AR71xx/AR913x based Mikrotik boards.
	e.g. MikroTik RB-4xx or RB-750
endef


