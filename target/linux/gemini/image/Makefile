#
# Copyright (C) 2009-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ifeq ($(SUBTARGET),wiligear)
define Image/Prepare
# WBD111: mach id 1690 (0x69a)
	echo -en "\x06\x1c\xa0\xe3\x9a\x10\x81\xe3" > $(KDIR)/wbd111-zImage
	cat $(KDIR)/zImage >> $(KDIR)/wbd111-zImage
# WBD222: mach id 2753 (0xAC1)
	echo -en "\x0a\x1c\xa0\xe3\xc1\x10\x81\xe3" > $(KDIR)/wbd222-zImage
	cat $(KDIR)/zImage >> $(KDIR)/wbd222-zImage
endef
endif

ifeq ($(SUBTARGET),raidsonic)
define Image/Prepare
# NAS4220: mach id 2038 (0x7F6)
	echo -en "\x07\x1c\xa0\xe3\xf6\x10\x81\xe3" > $(KDIR)/nas4220-zImage
	cat $(KDIR)/zImage >> $(KDIR)/nas4220-zImage
endef
endif

ifeq ($(SUBTARGET),wiligear)
define Image/BuildKernel
# workaround the bootloader's bug with extra nops
	echo -en "\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1" > $(BIN_DIR)/$(IMG_PREFIX)-wbd111-zImage
	cat $(KDIR)/wbd111-zImage >> $(BIN_DIR)/$(IMG_PREFIX)-wbd111-zImage
	echo -en "\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1" > $(BIN_DIR)/$(IMG_PREFIX)-wbd222-zImage
	cat $(KDIR)/wbd222-zImage >> $(BIN_DIR)/$(IMG_PREFIX)-wbd222-zImage
endef
endif

define Image/Build/jffs2-64k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=64k conv=sync
endef

define Image/Build/jffs2-128k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=128k conv=sync
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=128k conv=sync
endef

ifeq ($(SUBTARGET),wiligear)
define Image/Build
	$(call Image/Build/$(1),$(1))
	-$(STAGING_DIR_HOST)/bin/mkfwimage2 \
		-m GEOS -f 0x30000000 -z \
		-v WILI-S.WILIBOARD.v5.00.SL3512.OpenWrt.00000.000000.000000 \
		-o $(BIN_DIR)/$(IMG_PREFIX)-wbd111-$(1).bin \
		-p Kernel:0x020000:0x100000:0:0:$(BIN_DIR)/$(IMG_PREFIX)-wbd111-zImage \
		-p Ramdisk:0x120000:0x500000:0:0:$(BIN_DIR)/$(IMG_PREFIX)-$(1).img

	-$(STAGING_DIR_HOST)/bin/mkfwimage2 \
		-m GEOS -f 0x30000000 -z \
		-v WILI-S.WBD222.v5.00.SL3512.OpenWrt.00000.000000.000000 \
		-o $(BIN_DIR)/$(IMG_PREFIX)-wbd222-$(1).bin \
		-p Kernel:0x020000:0x100000:0:0:$(BIN_DIR)/$(IMG_PREFIX)-wbd222-zImage \
		-p Ramdisk:0x120000:0x500000:0:0:$(BIN_DIR)/$(IMG_PREFIX)-$(1).img
endef
endif

ifeq ($(SUBTARGET),raidsonic)
define Image/Build
	$(call Image/Build/$(1),$(1))
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img of=$(BIN_DIR)/rd.gz bs=6144k count=1
#	dd if=/dev/zero of=$(BIN_DIR)/hddapp.tgz bs=6144k count=1
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img of=$(BIN_DIR)/hddapp.tgz bs=6144k count=1 seek=1
	cp $(KDIR)/nas4220-zImage $(BIN_DIR)/$(IMG_PREFIX)-nas4220-zImage
	cp $(BIN_DIR)/$(IMG_PREFIX)-nas4220-zImage $(BIN_DIR)/zImage
	cp ./ImageInfo-ib4220 $(BIN_DIR)/ImageInfo
	(cd $(BIN_DIR); tar -czf $(IMG_PREFIX)-sysupgrade-ib4220.tar.gz ImageInfo zImage rd.gz hddapp.tgz)
	mv $(BIN_DIR)/rd.gz $(BIN_DIR)/$(IMG_PREFIX)-nas4220-rd.gz
	mv $(BIN_DIR)/hddapp.tgz $(BIN_DIR)/$(IMG_PREFIX)-nas4220-hddapp.tgz
	rm -f $(BIN_DIR)/zImage $(BIN_DIR)/ImageInfo
endef
endif

$(eval $(call BuildImage))
