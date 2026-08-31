ARCH:=mips
BOARDNAME:=en751221
CPU_TYPE:=24kc
KERNELNAME:=vmlinuz.bin

define Target/Description
	Build firmware images for EcoNet EN751221 family SoC, including
	EN7512, EN7513, EN7521, EN7526 and EN7586.
endef
