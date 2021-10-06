ARCH:=aarch64
SUBTARGET:=mt7622
BOARDNAME:=MT7622
CPU_TYPE:=cortex-a53
DEFAULT_PACKAGES += kmod-mt7615e kmod-mt7615-firmware wpad-basic-wolfssl blockdev uboot-envtools
KERNELNAME:=Image dtbs
KERNEL_PATCHVER:=5.10

define Target/Description
	Build firmware images for MediaTek MT7622 ARM based boards.
endef
