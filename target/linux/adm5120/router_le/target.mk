ARCH:=mipsel
SUBTARGET:=router_le
BOARDNAME:=Little Endian
FEATURES+=squashfs tgz

define Target/Description
	Build firmware images for Infineon/ADMtek ADM5120 based boards
	running in little-endian mode (e.g: RouterBoard RB1xx, Compex WP54x ...)
endef

