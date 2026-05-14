ARCH:=aarch64
SUBTARGET:=filogic
BOARDNAME:=Filogic 8x0 (MT798x)
CPU_TYPE:=generic
CPU_SUBTYPE:=cortex-a73
ARCH_PACKAGES:=aarch64_cortex-a53
CFLAGS:=-O2 -pipe -march=armv8-a+crypto+crc -mtune=cortex-a73
DEFAULT_PACKAGES += kmod-crypto-hw-safexcel kmod-mt7915e wpad-openssl uboot-envtools
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic ARM based boards.
endef
