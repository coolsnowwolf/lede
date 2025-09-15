BOARDNAME:=Devices with small flash
FEATURES += low_mem small_flash

DEFAULT_PACKAGES += wpad-basic-mbedtls

define Target/Description
	Build firmware images for Atheros AR71xx/AR913x/AR934x based boards with small flash
endef
