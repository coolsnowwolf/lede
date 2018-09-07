#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

JFFS2_BLOCKSIZE=256k 512k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ifneq ($(CONFIG_TARGET_xburst_qi_lb60),)
UBI_OPTS = -m 4096 -p 512KiB
UBIFS_OPTS = -m 4096 -e 516096 -c 4095
else
UBI_OPTS = -m 2048 -p 128KiB -s 512
UBIFS_OPTS = -m 2048 -e 126KiB -c 4096
endif

UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage.bin

define MkImageGzip
	gzip -9n -c $(1) > $(1).gz
	mkimage -A mips -O linux -T kernel -a 0x80010000 -C gzip \
		-e 0x80010000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(1).gz $(2)
endef

define Image/Prepare
	$(call MkImageGzip,$(KDIR)/vmlinux,$(KDIR)/uimage)
endef

define Image/BuildKernel
	cp $(KDIR)/uimage $(UIMAGE)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
endef

$(eval $(call BuildImage))
