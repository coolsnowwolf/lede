#
# Copyright (C) 2009-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

# Cook a "WRGG" image, this board is apparently one in the D-Link
# WRGG family and uses the exact same firmware format as other
# D-Link devices.
define Build/dir685-images
	mkwrggimg -i $(IMAGE_KERNEL) \
	-o $(BIN_DIR)/$(IMG_PREFIX)-dir685-fwupdate.bin \
	-d /dev/mtdblock/1 \
	-s wrgns01_dlwbr_dir685RBWW \
	-v 'N/A' \
	-m dir685 \
	-B 96bb
endef

# Build D-Link DNS-313 images using the special header tool.
# rootfs.tgz and rd.tgz contains nothing, we only need them
# to satisfy the boot loader on the device. The zImage is
# the only real content.
define Build/dns313-images
	if [ -d $(BIN_DIR)/.boot ] ; then rm -rf $(BIN_DIR)/.boot ; fi
	mkdir -p $(BIN_DIR)/.boot
	echo "dummy" > $(BIN_DIR)/.boot/dummyfile
	dns313-header $(BIN_DIR)/.boot/dummyfile \
		$(BIN_DIR)/.boot/rootfs.tgz
	dns313-header $(BIN_DIR)/.boot/dummyfile \
		$(BIN_DIR)/.boot/rd.gz
	dns313-header $(IMAGE_KERNEL) \
		$(BIN_DIR)/.boot/zImage
	rm -f $(BIN_DIR)/.boot/dummyfile
	(cd $(BIN_DIR); tar -czf $(IMG_PREFIX)-dns313-bootpart.tar.gz .boot)
	if [ -d $(BIN_DIR)/.boot ] ; then rm -rf $(BIN_DIR)/.boot ; fi
endef

# Create the special NAS4220B image format with the squashfs
# and overlay inside the "rd.gz" file. We pad it out to 6144K
# which is the size of the initramfs partition.
#
# The "application" partition is just blank. You can put anything
# there when using OpenWRT. We just use that to create the
# "sysupgrade" firmware image.
define Build/nas4220b-images
	dd if=$(IMAGE_ROOTFS) of=$(BIN_DIR)/rd.gz bs=6144k conv=sync
	dd if=/dev/zero of=$(BIN_DIR)/hddapp.tgz bs=6144k count=1
	cp $(IMAGE_KERNEL) $(BIN_DIR)/zImage
	cp ./ImageInfo-ib4220 $(BIN_DIR)/ImageInfo
	(cd $(BIN_DIR); tar -czf $(IMG_PREFIX)-sysupgrade-ib4220b.tar.gz ImageInfo zImage rd.gz hddapp.tgz)
	mv $(BIN_DIR)/rd.gz $(BIN_DIR)/$(IMG_PREFIX)-nas4220b-rd.gz
	rm $(BIN_DIR)/hddapp.tgz
	mv $(BIN_DIR)/zImage $(BIN_DIR)/$(IMG_PREFIX)-nas4220b-zImage
	rm -f $(BIN_DIR)/ImageInfo
endef

# The Itian Square One SQ201 uses the same method.
define Build/sq201-images
	dd if=$(IMAGE_ROOTFS) of=$(BIN_DIR)/rd.gz bs=6144k conv=sync
	dd if=/dev/zero of=$(BIN_DIR)/hddapp.tgz bs=6144k count=1
	cp $(IMAGE_KERNEL) $(BIN_DIR)/zImage
	cp ./ImageInfo-sq201 $(BIN_DIR)/ImageInfo
	sed -i -e "s/DATESTR/`date +%Y%m%d`/g" $(BIN_DIR)/ImageInfo
	(cd $(BIN_DIR); tar -czf $(IMG_PREFIX)-sysupgrade-sq201.tar.gz ImageInfo zImage rd.gz hddapp.tgz)
	mv $(BIN_DIR)/rd.gz $(BIN_DIR)/$(IMG_PREFIX)-sq201-rd.gz
	rm $(BIN_DIR)/hddapp.tgz
	mv $(BIN_DIR)/zImage $(BIN_DIR)/$(IMG_PREFIX)-sq201-zImage
	rm -f $(BIN_DIR)/ImageInfo
endef


# WBD-111 and WBD-222:
# work around the bootloader's bug with extra nops
# FIXME: is this really needed now that we no longer append the code
# to change the machine ID number? Needs testing on Wiliboard.
define Build/wbd-nops
	mv $@ $@.tmp
	echo -en "\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1\x00\x00\xa0\xe1" > $@
	cat $@.tmp >> $@
	rm -f $@.tmp
endef

# All DTB files are prefixed with "gemini-"
define Device/Default
	DEVICE_DTS := $(patsubst %.dtb,%,$(notdir $(wildcard $(if $(IB),$(KDIR),$(DTS_DIR))/*-$(1).dtb)))
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_NAME := zImage
	KERNEL := kernel-bin | append-dtb
	FILESYSTEMS := squashfs
	IMAGE_NAME := $$(IMAGE_PREFIX)-$$(1).$$(2)
	BLOCKSIZE := 128k
	PAGESIZE := 2048
endef

# A reasonable set of default packages handling the NAS type
# of devices out of the box (former NAS42x0 IcyBox defaults)
GEMINI_NAS_PACKAGES:=kmod-md-mod kmod-md-linear kmod-md-multipath \
		kmod-md-raid0 kmod-md-raid1 kmod-md-raid10 kmod-md-raid456 \
		kmod-fs-btrfs kmod-fs-cifs kmod-fs-nfs \
		kmod-fs-nfsd kmod-fs-ntfs kmod-fs-reiserfs kmod-fs-vfat \
		kmod-nls-utf8 kmod-usb-storage-extras \
		samba36-server mdadm cfdisk fdisk e2fsprogs badblocks

define Device/dlink-dir-685
	DEVICE_TITLE := D-Link DIR-685 Xtreme N Storage Router
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES) \
			kmod-switch-rtl8366rb swconfig \
			rt2800-pci
	IMAGES += dir685-image
	IMAGE/dir685-image := dir685-images
endef
TARGET_DEVICES += dlink-dir-685

define Device/dlink-dns-313
	DEVICE_TITLE := D-Link DNS-313 1-Bay Network Storage Enclosure
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES)
	IMAGES += dns313-image
	IMAGE/dns313-image := dns313-images
endef
TARGET_DEVICES += dlink-dns-313

define Device/nas4220b
	DEVICE_TITLE := Raidsonic NAS IB-4220-B
	IMAGES += nas4220b-image
	IMAGE/nas4220b-image := nas4220b-images
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES)
endef
TARGET_DEVICES += nas4220b

define Device/rut1xx
	DEVICE_TITLE := Teltonika RUT1xx
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES)
endef
TARGET_DEVICES += rut1xx

define Device/sq201
	DEVICE_TITLE := ITian Square One SQ201
	IMAGES += sq201-image
	IMAGE/sq201-image := sq201-images
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES) rt61-pci usb2-pci
endef
TARGET_DEVICES += sq201

define Device/wbd111
	DEVICE_TITLE := Wiliboard WBD-111
	KERNEL := kernel-bin | append-dtb | wbd-nops
endef
TARGET_DEVICES += wbd111

define Device/wbd222
	DEVICE_TITLE := Wiliboard WBD-222
	KERNEL := kernel-bin | append-dtb | wbd-nops
endef
TARGET_DEVICES += wbd222

$(eval $(call BuildImage))
