#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

# UML only makes sense on linux
ifeq ($(HOST_OS),Linux)

ARCH:=$(shell uname -m | sed \
	-e 's/i[3-9]86/i386/' \
	-e 's/mipsel/mips/' \
	-e 's/mipseb/mips/' \
	-e 's/powerpc/ppc/' \
	-e 's/sh[234]/sh/' \
	-e 's/armeb/arm/' \
)
BOARD:=uml
BOARDNAME:=User Mode Linux
FEATURES:=squashfs ext4 audio source-only

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

define Kernel/Patch
	mkdir -p $(LINUX_DIR)/arch/um/include/uapi
	mv $(LINUX_DIR)/arch/um/include/asm $(LINUX_DIR)/arch/um/include/uapi/
	$(Kernel/Patch/Default)
endef

LINUX_TARGET_CONFIG:=$(CURDIR)/config/$(ARCH)

DEFAULT_PACKAGES += wpad-basic kmod-mac80211-hwsim mkf2fs e2fsprogs

endif

$(eval $(call BuildTarget))
