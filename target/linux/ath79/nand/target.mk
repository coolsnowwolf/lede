BOARDNAME := Generic devices with NAND flash
FEATURES += squashfs nand rtc

define Target/Description
	Build firmware for Atheros AR71xx/AR913x based boards with
	NAND flash, e.g. Netgear WNDR4300.
endef
