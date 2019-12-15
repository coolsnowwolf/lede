#
# Copyright (C) 2009-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/copy-kernel.bin
	$(call locked,$(MAKE) -C copy-kernel CROSS_COMPILE=$(TARGET_CROSS) O=$(KDIR),gemini-copy-kernel.bin)
endef

# Cook a "WRGG" image, this board is apparently one in the D-Link
# WRGG family and uses the exact same firmware format as other
# D-Link devices.
define Build/dir685-image
	mkwrggimg -i $@ \
	-o $@.new \
	-d /dev/mtdblock/1 \
	-s wrgns01_dlwbr_dir685RBWW \
	-v 'N/A' \
	-m dir685 \
	-B 96bb

	mv $@.new $@
endef

# Padding added after the rootfs to an even 128k boundary
# as this is 128k eraseblocks flash.
define Build/dir685-pad-rootfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(IMAGE_ROOTFS) -c 128 >>$@
endef

# Build D-Link DNS-313 images using the special header tool.
# rootfs.tgz and rd.tgz contains nothing, we only need them
# to satisfy the boot loader on the device. The zImage is
# the only real content.
define Build/dns313-images
	mkdir -p $@.tmp/.boot
	chmod 755 $@.tmp/.boot

	echo "dummy" > $@.tmp/dummyfile

	dns313-header $@.tmp/dummyfile \
		$@.tmp/.boot/rootfs.tgz
	dns313-header $@.tmp/dummyfile \
		$@.tmp/.boot/rd.gz
	dns313-header $(IMAGE_KERNEL) \
		$@.tmp/.boot/zImage

	rm $@.tmp/dummyfile

	genext2fs --block-size $(BLOCKSIZE:%k=%Ki) \
		--size-in-blocks $$((1024 * $(CONFIG_TARGET_KERNEL_PARTSIZE))) \
		--squash-uids \
		--root $@.tmp $@.tmp-boot

	# The device firmware needs revision 1 of EXT2
	tune2fs -O filetype $@.tmp-boot
	e2fsck -pDf $@.tmp-boot > /dev/null

	./dns313_gen_hdd_img.sh $@ $@.tmp-boot $(IMAGE_ROOTFS) \
		$(CONFIG_TARGET_KERNEL_PARTSIZE) \
		$(CONFIG_TARGET_ROOTFS_PARTSIZE)

	rm -rf $@.tmp
endef

define Build/wiligear-image
	$(STAGING_DIR_HOST)/bin/mkfwimage2 \
		-m GEOS -f 0x30000000 -z \
		-v $(1).v5.00.SL3512.OpenWrt.00000.000000.000000 \
		-p Kernel:0x020000:0x100000:0:0:$(IMAGE_KERNEL) \
		-p Ramdisk:0x120000:0x500000:0:0:$@ \
		-o $@.new

	mv $@.new $@
endef

# Create the default image format used by the StorLink reference design
# SL93512r, Raidsonic NAS4220B and Itian Square One SQ201
# with the squashfs and overlay inside the "application" partition.
#
# These devices have a hard-coded partition table that the boot loader
# constantly reflashes back, so we need to work around it like this:
#
# 0x000000120000-0x000000320000 : "Kern" - small copy routine and first
#                                 part of the kernel goes here
# 0x000000320000-0x000000920000 : "Ramdisk" - second part of the kernel and
#                                 some padding goes here
# 0x000000920000-0x000000f20000 : "Application" - rootfs goes here
define Build/storlink-default-images
	mkdir -p $@.tmp

	# "App" partition is the rootfs
	mv $@ $@.tmp/hddapp.tgz
	# 256 bytes copy routine
	dd if=$(KDIR)/copy-kernel.bin of=$@.tmp/zImage
	$(call Image/pad-to,$@.tmp/zImage,512)
	# Copy first part of the kernel into zImage
	dd if=$(IMAGE_KERNEL) of=$@.tmp/zImage bs=1 seek=512 count=2096640
	# Put the rest of the kernel into the "ramdisk"
	dd if=$(IMAGE_KERNEL) of=$@.tmp/rd.gz bs=1 skip=2096640 count=6144k conv=sync
	cp ./ImageInfo-$(1) $@.tmp/ImageInfo

	sed -i -e "s/DATESTR/`date +%Y%m%d $(if $(SOURCE_DATE_EPOCH),--date "@$(SOURCE_DATE_EPOCH)")`/g" $@.tmp/ImageInfo

	(cd $@.tmp; tar --sort=name --owner=0 --group=0 --numeric-owner -czf $@ * \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)"))

	rm -rf $@.tmp
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
	PROFILES := Default
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_NAME := zImage
	KERNEL := kernel-bin | append-dtb
	BLOCKSIZE := 128k
	SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
endef

# A reasonable set of default packages handling the NAS type
# of devices out of the box (former NAS42x0 IcyBox defaults)
GEMINI_NAS_PACKAGES:=kmod-md-mod kmod-md-linear kmod-md-multipath \
		kmod-md-raid0 kmod-md-raid1 kmod-md-raid10 kmod-md-raid456 \
		kmod-fs-btrfs kmod-fs-cifs kmod-fs-nfs \
		kmod-fs-nfsd kmod-fs-ntfs kmod-fs-reiserfs kmod-fs-vfat \
		kmod-nls-utf8 kmod-usb-storage-extras \
		samba36-server mdadm cfdisk fdisk e2fsprogs badblocks \
		partx-utils

# The DIR-685 flash layout is kernel in WRGG format, padded and followed
# by the appended rootfs followed by some reasonable JFFS padding, the
# remainder will be used by JFFS2 through overlayfs.
#
# - For the factory image, the WRGG image includes the rootfs so that the
#   default firmware will flash it properly as all it knows is WRGG format.
# - For the sysupgrade, we do not include the rootfs in the kernel image
#   so it is not needelessly tossed into the RAM by the boot loader.
#   This will be flashed from OpenWrt userland anyways so we only need
#   the minimum to make the boot loader happy.
define Device/dlink_dir-685
	DEVICE_VENDOR := D-Link
	DEVICE_MODEL := DIR-685 Xtreme N Storage Router
	DEVICE_DTS := gemini-dlink-dir-685
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES) \
			kmod-rt2800-pci
	IMAGES := factory.bin sysupgrade.bin
	# Pad to 128k erase blocks with 160 bytes WRGG header
	IMAGE/factory.bin := append-kernel | pad-offset 128k 160 | append-rootfs | dir685-pad-rootfs | dir685-image
	IMAGE/sysupgrade.bin := append-kernel | pad-offset 128k 160 | dir685-image | append-rootfs | dir685-pad-rootfs | append-metadata
endef
TARGET_DEVICES += dlink_dir-685

define Device/dlink_dns-313
	DEVICE_VENDOR := D-Link
	DEVICE_MODEL := DNS-313 1-Bay Network Storage Enclosure
	DEVICE_DTS := gemini-dlink-dns-313
	DEVICE_TYPE := nas
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES)
	BLOCKSIZE := 1k
	FILESYSTEMS := ext4
	IMAGES := factory.bin.gz
	IMAGE/factory.bin.gz := dns313-images | gzip
endef
TARGET_DEVICES += dlink_dns-313

# Default images setup used by the StorLink reference designs
define Device/storlink-reference
	COMPILE := copy-kernel-$(1).bin
	COMPILE/copy-kernel-$(1).bin := copy-kernel.bin
	IMAGES := factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | pad-to 6144k | \
		storlink-default-images $(1)
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES)
endef

define Device/itian_sq201
	$(Device/storlink-reference)
	DEVICE_VENDOR := ITian
	DEVICE_MODEL := Square One SQ201
	DEVICE_DTS := gemini-sq201
	DEVICE_PACKAGES += kmod-rt61-pci kmod-usb2-pci
endef
TARGET_DEVICES += itian_sq201

define Device/raidsonic_ib-4220-b
	$(Device/storlink-reference)
	DEVICE_VENDOR := Raidsonic
	DEVICE_MODEL := NAS IB-4220-B
	DEVICE_DTS := gemini-nas4220b
	DEVICE_TYPE := nas
endef
TARGET_DEVICES += raidsonic_ib-4220-b

define Device/storlink_sl93512r
	$(Device/storlink-reference)
	DEVICE_VENDOR := StorLink
	DEVICE_MODEL := SL93512r
	DEVICE_DTS := gemini-sl93512r
endef
TARGET_DEVICES += storlink_sl93512r


# The wiliboard images need some changes to be functional and buildable.
#
# The dts would need to use the ecoscentric,redboot-fis-partitions partition
# parser to get the correct partition offsets and size.
#
# The mkfwimage2 call need to be adjusted to reflect the real size of kernel
# and rootfs. It is expected that the OEM firmware adjusts the on flash
# partition table with the values defined in the image header.
define Device/wiliboard_wbd111
	DEVICE_VENDOR := Wiliboard
	DEVICE_MODEL := WBD-111
	DEVICE_DTS := gemini-wbd111
	KERNEL := kernel-bin | append-dtb | wbd-nops
	IMAGES := factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | wiligear-image "WILI-S.WILIBOARD"
endef

define Device/wiliboard_wbd222
	DEVICE_VENDOR := Wiliboard
	DEVICE_MODEL := WBD-222
	DEVICE_DTS := gemini-wbd222
	KERNEL := kernel-bin | append-dtb | wbd-nops
	IMAGES := factory.bin
	IMAGE/factory.bin := append-rootfs | pad-rootfs | wiligear-image "WILI-S.WBD222"
endef

$(eval $(call BuildImage))
