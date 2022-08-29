ARCH:=aarch64
SUBTARGET:=filogic
BOARDNAME:=Filogic 830 (MT7986)
CPU_TYPE:=cortex-a53
DEFAULT_PACKAGES += kmod-crypto-hw-safexcel kmod-mt7915e kmod-mt7986-firmware wpad-basic-wolfssl uboot-envtools
KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek Filogic ARM based boards.
endef
