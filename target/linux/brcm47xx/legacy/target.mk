FEATURES += low_mem pcmcia small_flash
BOARDNAME:=Legacy (BMIPS3300)

DEFAULT_PACKAGES += wpad-mini

define Target/Description
	Build firmware for Broadcom BCM47xx and BCM53xx devices with
	BMIPS3300 CPU except for BCM4705 SoC.
	Supported SoCs: BCM5352E, BCM5354, BCM5365?, BCM4712, BCM4704.
endef
