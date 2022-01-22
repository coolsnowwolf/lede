#
# Copyright (C) 2009 OpenWrt.org
#

SUBTARGET:=m3
BOARDNAME:=Meson3 based boards
ARCH_PACKAGES:=
FEATURES+=
CPU_TYPE:=cortex-a9
CPU_SUBTYPE:=neon
INITRAMFS_BIN:=$(PLATFORM_SUBDIR)/m3-rootfs.cpio.gz

define Kernel/Patch/Amlogic
	$(call PatchDir,$(PKG_BUILD_DIR),patches-m3-3.10,platform/)
endef

define Target/Description
	Build firmware images for Meson3 based boards.
endef

