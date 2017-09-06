BOARDNAME:=Devices with NAND flash (Routers)
FEATURES += nand pcie ramdisk squashfs usb

DEFAULT_PACKAGES += kmod-ath9k swconfig wpad-mini

define Target/Description
	Build firmware images for APM821XX boards with NAND flash.
	For routers like the MR24 or the WNDR4700.
endef
