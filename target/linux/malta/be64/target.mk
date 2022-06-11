ARCH:=mips64
CPU_TYPE:=mips64r2
SUBTARGET:=be64
FEATURES+=source-only
BOARDNAME:=Big Endian (64-bits)

define Target/Description
	Build BE firmware images for MIPS Malta CoreLV board running in
	big-endian and 64-bits mode
endef
