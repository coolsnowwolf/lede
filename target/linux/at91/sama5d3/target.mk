BOARDNAME:=SAMA5D3 (Cortex-A5)
CPU_TYPE:=cortex-a5

DEFAULT_PACKAGES += kmod-usb2

define Target/Description
	Build generic firmware for SAMA5D3 AT91 platforms
	using the ARMv7 instruction set.
endef
