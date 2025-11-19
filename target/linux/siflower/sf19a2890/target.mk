ARCH:=mipsel
SUBTARGET:=sf19a2890
BOARDNAME:=Siflower SF19A2890 based boards
FEATURES+=fpu
CPU_TYPE:=24kc
CPU_SUBTYPE:=24kf

KERNELNAME:=vmlinux

define Target/Description
	Build firmware images for Siflower SF19A2890 based boards.
endef
