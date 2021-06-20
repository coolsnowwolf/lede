# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=mips
CPU_TYPE:=4kec
BOARD:=realtek
BOARDNAME:=Realtek MIPS
DEVICE_TYPE:=basic
FEATURES:=ramdisk squashfs
SUBTARGETS:=generic

KERNEL_PATCHVER:=5.4

define Target/Description
	Build firmware images for Realtek RTL83xx based boards.
endef

include $(INCLUDE_DIR)/target.mk

FEATURES := $(filter-out mips16,$(FEATURES))

DEFAULT_PACKAGES += uboot-envtools ethtool kmod-gpio-button-hotplug \
	dnsmasq firewall ip6tables iptables odhcp6c odhcpd-ipv6only \
	ip-full ip-bridge tc

$(eval $(call BuildTarget))
