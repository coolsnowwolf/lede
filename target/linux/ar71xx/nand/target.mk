BOARDNAME := Generic devices with NAND flash
FEATURES += squashfs nand rtc small_flash

define Target/Description
	Build firmware images for Atheros AR71xx/AR913x based boards with
	NAND flash, e.g. Netgear WNDR4300.
endef
