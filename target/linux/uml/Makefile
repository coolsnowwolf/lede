# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2021 OpenWrt.org

include $(TOPDIR)/rules.mk

# UML only makes sense on linux
ifeq ($(HOST_OS),Linux)
  ifeq ($(HOST_ARCH),x86_64)

ARCH:=x86_64
BOARD:=uml
BOARDNAME:=User Mode Linux
FEATURES:=audio ext4 rootfs-part squashfs

KERNEL_PATCHVER:=5.10

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += wpad-basic-wolfssl kmod-mac80211-hwsim mkf2fs e2fsprogs

  endif
endif

$(eval $(call BuildTarget))
