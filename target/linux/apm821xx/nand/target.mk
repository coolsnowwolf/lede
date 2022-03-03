BOARDNAME:=Devices with NAND flash (Routers)
FEATURES += nand pcie

DEFAULT_PACKAGES += kmod-ath9k swconfig wpad-basic-wolfssl

define Target/Description
	Build firmware images for APM821XX boards with NAND flash.
	For routers like the MR24 or the WNDR4700.
endef
