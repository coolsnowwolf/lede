BOARDNAME:=SAMA5 boards(Cortex-A5)
CPU_TYPE:=cortex-a5
CPU_SUBTYPE:=vfpv4
FEATURES+=fpu
DEFAULT_PACKAGES += kmod-usb2

define Target/Description
	Build generic firmware for Microchip(Atmel AT91) SAMA5D2,
	SAMA5D3 and SAMA5D4 MPU's using the ARMv7 instruction set.
endef
