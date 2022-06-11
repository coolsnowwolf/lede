ARCH:=mipsel
CPU_TYPE:=24kc
SUBTARGET:=le
FEATURES+=source-only
BOARDNAME:=Little Endian

define Target/Description
	Build LE firmware images for MIPS Malta CoreLV board running in
	little-endian mode
endef
