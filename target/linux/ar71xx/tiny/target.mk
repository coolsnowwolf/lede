BOARDNAME:=Devices with small flash
FEATURES += squashfs small_flash

DEFAULT_PACKAGES += wpad-mini

define Target/Description
	Build firmware images for Atheros AR71xx/AR913x/AR934x based boards with small NOR flash.
endef


