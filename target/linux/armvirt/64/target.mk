ARCH:=aarch64
SUBTARGET:=64
BOARDNAME:=QEMU ARMv8 Virtual Machine (cortex-a53)
CPU_TYPE:=cortex-a53
KERNELNAME:=Image

define Target/Description
  Build multi-platform images for the ARMv8 instruction set architecture
endef
