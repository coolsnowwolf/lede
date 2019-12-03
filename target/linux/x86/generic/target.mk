BOARDNAME:=x86 32bit
CPU_TYPE :=core2
FEATURES += audio pci pcie usb
DEFAULT_PACKAGES += kmod-button-hotplug

define Target/Description
	Build firmware images for modern x86 based boards with CPUs
	supporting at least theIntel Core 2 CPU instruction set with
	MMX, SSE, SSE2, SSE3 and SSSE3.
endef

