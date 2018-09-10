ARCH:=mips
SUBTARGET:=router_be
BOARDNAME:=Big Endian
FEATURES+=squashfs

define Target/Description
	Build firmware images for Infineon/ADMTek ADM5120 based boards
	running in big-endian mode (e.g : ZyXEL Prestige 335WT ...)
endef

