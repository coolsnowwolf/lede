ARCH:=aarch64
SUBTARGET:=armv8
CPU_TYPE:=cortex-a55
BOARDNAME:=RK356x boards (64 bit)

define Target/Description
	Build firmware image for Rockchip RK33xx devices.
	This firmware features a 64 bit kernel.
endef
