ARCH:=arm
SUBTARGET:=32
BOARDNAME:=32-bit ARM QEMU Virtual Machine
CPU_TYPE:=cortex-a15
CPU_SUBTYPE:=neon-vfpv4
KERNELNAME:=zImage

define Target/Description
  Build images for $(BOARDNAME)
endef
