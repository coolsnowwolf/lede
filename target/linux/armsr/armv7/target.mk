ARCH:=arm
SUBTARGET:=armv7
BOARDNAME:=32-bit (armv7) machines
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
KERNELNAME:=zImage

define Target/Description
  Build images for $(BOARDNAME)
endef
