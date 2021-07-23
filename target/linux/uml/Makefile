# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

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

KERNEL_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk

LINUX_TARGET_CONFIG:=$(CURDIR)/config/$(ARCH)

DEFAULT_PACKAGES += wpad-basic-wolfssl kmod-mac80211-hwsim mkf2fs e2fsprogs

endif

$(eval $(call BuildTarget))
