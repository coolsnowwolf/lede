ARCH:=riscv64
SUBTARGET:=sf21
BOARDNAME:=Siflower SF21A6826/SF21H8898 based boards
FEATURES+=fpu nand separate_ramdisk
DEFAULT_PACKAGES += fitblk
KERNELNAME:=Image

define Target/Description
	Build firmware images for Siflower SF21A6826/SF21H8898 based boards.
endef
