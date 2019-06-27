#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=malta
BOARDNAME:=MIPS Malta CoreLV board (qemu)
CPU_TYPE:=24kc
SUBTARGETS:=le be le64 be64
INITRAMFS_EXTRA_FILES:=
MAINTAINER:=Florian Fainelli <florian@openwrt.org>
FEATURES:=ramdisk

KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += wpad-basic kmod-mac80211-hwsim kmod-pcnet32

$(eval $(call BuildTarget))
