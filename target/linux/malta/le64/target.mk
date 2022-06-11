ARCH:=mips64el
CPU_TYPE:=mips64r2
SUBTARGET:=le64
FEATURES+=source-only
BOARDNAME:=Little Endian (64-bits)

define Target/Description
	Build LE firmware images for MIPS Malta CoreLV board running in
	little-endian and 64-bits mode
endef
