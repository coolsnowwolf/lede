# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2016 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

USB1_PACKAGES := kmod-usb-ohci
USB2_PACKAGES := $(USB1_PACKAGES) kmod-usb2

define Build/Clean
	$(MAKE) -C lzma-loader clean
endef

define Image/Prepare
	# Optimized LZMA compression (with dictionary), handled by lzma-loader.
	cat $(KDIR)/vmlinux | $(STAGING_DIR_HOST)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $(KDIR)/vmlinux.lzma

	# Less optimal LZMA compression (no dictionary), handled by CFE.
	$(STAGING_DIR_HOST)/bin/lzma e -so -d16 $(KDIR)/vmlinux > $(KDIR)/vmlinux-nodictionary.lzma

	gzip -nc9 $(KDIR)/vmlinux > $(KDIR)/vmlinux.gz
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	cat $(KDIR)/vmlinux-initramfs | $(STAGING_DIR_HOST)/bin/lzma e -si -so -eos -lc1 -lp2 -pb2 > $(KDIR)/vmlinux-initramfs.lzma
	$(STAGING_DIR_HOST)/bin/lzma e -so -d16 $(KDIR)/vmlinux-initramfs > $(KDIR)/vmlinux-initramfs-nodictionary.lzma
endif
	rm -f $(KDIR)/loader.gz
	$(MAKE) -C lzma-loader \
		BUILD_DIR="$(KDIR)" \
		TARGET="$(KDIR)" \
		clean install
	echo -ne "\\x00" >> $(KDIR)/loader.gz
	rm -f $(KDIR)/fs_mark
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/fs_mark
	$(call prepare_generic_squashfs,$(KDIR)/fs_mark)
endef

define trxalign/jffs2-128k
-a 0x20000 -f $(KDIR)/root.$(1)
endef
define trxalign/jffs2-64k
-a 0x10000 -f $(KDIR)/root.$(1)
endef
define trxalign/squashfs
-a 1024 -f $(1) $(if $(2),-f $(2)) -a 0x10000 -A $(KDIR)/fs_mark
endef

#################################################
# Images
#################################################

define Build/trx-with-loader
	$(STAGING_DIR_HOST)/bin/trx \
		-m 33554432 \
		-o $@.new \
		-f $(KDIR)/loader.gz \
		-f $(IMAGE_KERNEL) \
		$(call trxalign/$(FILESYSTEM),$@)
	mv $@.new $@
endef

define Build/trx-v2-with-loader
	$(STAGING_DIR_HOST)/bin/trx \
		-2 \
		-m 33554432 \
		-o $@.new \
		-f $(KDIR)/loader.gz \
		-f $(KDIR)/vmlinux.lzma \
		$(call trxalign/$(FILESYSTEM),$@,$@.pattern)
	mv $@.new $@
endef

define Build/trx-without-loader
	$(STAGING_DIR_HOST)/bin/trx \
		-m 33554432 \
		-o $@.new \
		-f $(IMAGE_KERNEL) \
		$(call trxalign/$(FILESYSTEM),$@)
	mv $@.new $@
endef

define Build/asus-trx
	$(STAGING_DIR_HOST)/bin/asustrx -p $(PRODUCTID) -i $@ -o $@.new
	mv $@.new $@
endef

define Build/edimax-bin
	$(STAGING_DIR_HOST)/bin/trx2edips $@ $@.new
	mv $@.new $@
endef

define Build/huawei-bin
	dd if=/dev/zero of=$@.new bs=92 count=1
	echo -ne 'HDR0\x08\x00\x00\x00' >> $@.new
	cat $@ >> $@.new
	mv $@.new $@
endef

define Build/linksys-bin
	$(STAGING_DIR_HOST)/bin/addpattern -4 -p $(DEVICE_ID) -v v$(VERSION) $(if $(SERIAL),-s $(SERIAL)) -i $@ -o $@.new
	mv $@.new $@
endef

define Build/linksys-pattern-partition
	$(STAGING_DIR_HOST)/bin/addpattern -5 -p $(DEVICE_ID) -v v$(VERSION) $(if $(SERIAL),-s $(SERIAL)) -i /dev/null -o $@.pattern
endef

define Build/motorola-bin
	$(STAGING_DIR_HOST)/bin/motorola-bin -$(MOTOROLA_DEVICE) $@ $@.new
	mv $@.new $@
endef

define Build/prepend-with-elf
	mv $@ $@.old
	dd if=$(KDIR)/loader.elf of=$@ bs=131072 conv=sync
	cat $@.old >> $@
endef

define Build/tailed-bin
	echo $(BIN_TAIL) >> $@
endef

define Build/usrobotics-bin
	$(STAGING_DIR_HOST)/bin/trx2usr $@ $@.new
	mv $@.new $@
endef

#################################################
# Devices
#################################################

DEVICE_VARS += PRODUCTID
DEVICE_VARS += DEVICE_ID VERSION SERIAL
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_REGION
DEVICE_VARS += MOTOROLA_DEVICE
DEVICE_VARS += BIN_TAIL

define Device/Default
	KERNEL := kernel-bin
	IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1).$$(2)
	KERNEL_NAME = vmlinux.lzma
	KERNEL_INITRAMFS_NAME = vmlinux-initramfs.lzma
	FILESYSTEMS := $(FS_64K)
	IMAGES := trx
	IMAGE/trx := append-rootfs | trx-with-loader
endef

define Device/standard
  DEVICE_TITLE := Image with LZMA loader and LZMA compressed kernel
endef

define Device/standard-noloader-gz
  DEVICE_TITLE := Image with gzipped kernel
  KERNEL_NAME = vmlinux.gz
  IMAGE/trx := append-rootfs | trx-without-loader
endef

define Device/standard-noloader-nodictionarylzma
  DEVICE_TITLE := Image with LZMA compressed kernel matching CFE decompressor
  KERNEL_NAME = vmlinux-nodictionary.lzma
  IMAGE/trx := append-rootfs | trx-without-loader
endef

define Device/asus
	DEVICE_VENDOR := ASUS
	IMAGES := trx
	IMAGE/trx := append-rootfs | trx-with-loader | asus-trx
endef

define Device/linksys
	DEVICE_VENDOR := Linksys
	IMAGES := bin
	IMAGE/bin := append-rootfs | trx-with-loader | linksys-bin
endef

define Device/motorola
	DEVICE_VENDOR := Motorola
	IMAGES := bin
	IMAGE/bin := append-rootfs | trx-with-loader | motorola-bin
endef

define Device/netgear
	DEVICE_VENDOR := NETGEAR
	IMAGES := chk
	IMAGE/chk := append-rootfs | trx-with-loader | netgear-chk
endef

#################################################
# Subtarget devices
#################################################

include $(SUBTARGET).mk

#################################################
# Shared BuildImage defines
#################################################

define Image/Build/Initramfs
	$(STAGING_DIR_HOST)/bin/trx \
		-m 33554432 \
		-o $(BIN_DIR)/$(IMG_PREFIX)-initramfs.trx \
		-f $(KDIR)/loader.gz \
		-f $(KDIR)/vmlinux-initramfs.lzma
	$(STAGING_DIR_HOST)/bin/trx \
		-m 33554432 \
		-o $(BIN_DIR)/$(IMG_PREFIX)-initramfs-noloader-nodictionary.trx \
		-f $(KDIR)/vmlinux-initramfs-nodictionary.lzma
endef

# $(1): filesystem type.
define Image/Build
	# TODO: Move it to Device/*
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(call Image/Build/Initramfs)
endif
endef

$(eval $(call BuildImage))
