# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2009-2018 OpenWrt.org

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
	$(STAGING_DIR_HOST)/bin/tune2fs -O filetype $@.tmp-boot
	# Ignore errors because file system was intentionally broken with tune2fs
	-$(STAGING_DIR_HOST)/bin/e2fsck -pDf $@.tmp-boot > /dev/null

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

define CreateStorlinkProlog
	# 512 bytes copy routine
	dd if=$(KDIR)/copy-kernel-$(2).bin of=$(1)
	# Add OpenWrt prolog header (used by partition splitter)
	echo "OPENWRT-PROLOG-512" >> $(1)
	stat -c %s ${IMAGE_KERNEL} >> $(1)
	$(call Image/pad-to,$(1),512)
endef

define CreateStorlinkFactoryfile
	mkdir -p $@.tmp
	$(call CreateStorlinkProlog,$@.tmp/zImage,$(2))
	# Copy first part of the kernel into zImage
	dd if=$(IMAGE_KERNEL) of=$@.tmp/zImage bs=1 seek=512 count=$(3)
	$(call Image/pad-to,$@.tmp/zImage,128k)
	# Put the rest of the kernel into the "ramdisk"
	dd if=$(IMAGE_KERNEL) of=$@-ramdisk bs=1 skip=$(3) conv=sync
	$(call Image/pad-to,$@-ramdisk,128k)
	# Append the root filesystem right after this
	dd if=$(IMAGE_ROOTFS) >> $@-ramdisk
	$(STAGING_DIR_HOST)/bin/padjffs2 $(IMAGE_ROOTFS) -c 128 >> $@-ramdisk
	# Now rd.gz is too big so split off rd.gz and the tail into "hddapp.gz"
	dd if=$@-ramdisk of=$@.tmp/rd.gz bs=1 count=$(4) conv=sync
	dd if=$@-ramdisk of=$@.tmp/hddapp.tgz bs=1 skip=$(4) count=$(5) conv=sync
	rm -f $@-ramdisk
	# Taglabel
	cp ./ImageInfo-$(1) $@.tmp/ImageInfo

	sed -i -e "s/DATESTR/`date +%Y%m%d $(if $(SOURCE_DATE_EPOCH),--date "@$(SOURCE_DATE_EPOCH)")`/g" $@.tmp/ImageInfo

	(cd $@.tmp; tar --sort=name --owner=0 --group=0 --numeric-owner -czf $@ * \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)"))

	rm -rf $@.tmp
	exit 0
endef

define CreateStorlinkSysupgradefile
	$(call CreateStorlinkProlog,$@,$(2))
	# Catenate the kernel
	dd if=$(IMAGE_KERNEL) >> $@
	$(call Image/pad-to,$@,128k)
	# Append the root filesystem right after this
	dd if=$(IMAGE_ROOTFS) >> $@
	$(STAGING_DIR_HOST)/bin/padjffs2 $(IMAGE_ROOTFS) -c 128 >> $@
endef

# 1024k "Kern" partition, 5120k "Ramdisk" partition
define Build/storlink-1024k-factory-image
	$(call CreateStorlinkFactoryfile,$(1),1024k,1048064,5120k,1664k)
endef

define Build/storlink-1024k-sysupgrade-image
	$(call CreateStorlinkSysupgradefile,$(1),1024k)
endef

# 2048k "Kern" partition, 6144k "Ramdisk" partition
define Build/storlink-2048k-factory-image
	$(call CreateStorlinkFactoryfile,$(1),2048k,2096640,6144k,6144k)
endef

define Build/storlink-2048k-sysupgrade-image
	$(call CreateStorlinkSysupgradefile,$(1),2048k)
endef

# 3072k "Kern" partition, 6144k "Ramdisk" partition
define Build/storlink-3072k-factory-image
	$(call CreateStorlinkFactoryfile,$(1),3072k,3145216,6144k,6144k)
endef

define Build/storlink-3072k-sysupgrade-image
	$(call CreateStorlinkSysupgradefile,$(1),3072k)
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
	DEVICE_DTS_DIR = $$(DTS_DIR)/gemini
	KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
	KERNEL_NAME := zImage
	KERNEL := kernel-bin | append-dtb
	BLOCKSIZE := 128k
endef

# A reasonable set of default packages handling the NAS type
# of devices out of the box (former NAS42x0 IcyBox defaults)
GEMINI_NAS_PACKAGES := $(DEFAULT_PACKAGES.nas) \
		kmod-md-mod kmod-md-linear \
		kmod-md-raid0 kmod-md-raid1 kmod-md-raid10 kmod-md-raid456 \
		kmod-fs-btrfs kmod-fs-cifs kmod-fs-nfs \
		kmod-fs-nfsd kmod-fs-ntfs3 kmod-fs-reiserfs kmod-fs-vfat \
		kmod-nls-utf8 kmod-usb-storage-extras kmod-hwmon-drivetemp \
		cfdisk e2fsprogs badblocks \
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
			kmod-dsa-rtl8366rb
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
	DEVICE_PACKAGES := $(GEMINI_NAS_PACKAGES) usbgadget-acm
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
	IMAGES := factory.bin sysupgrade.bin
	FILESYSTEMS := squashfs
endef

define Device/itian_sq201
	$(Device/storlink-reference)
	DEVICE_VENDOR := ITian
	DEVICE_MODEL := Square One SQ201
	DEVICE_DTS := gemini-sq201
	IMAGE/factory.bin := storlink-2048k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-2048k-sysupgrade-image $(1) |\
				append-metadata
	DEVICE_PACKAGES += $(GEMINI_NAS_PACKAGES) kmod-rt61-pci \
		kmod-usb2-pci kmod-dsa-vsc73xx-spi
endef
TARGET_DEVICES += itian_sq201

define Device/raidsonic_ib-4210-b
	$(Device/storlink-reference)
	DEVICE_VENDOR := Raidsonic
	DEVICE_MODEL := NAS IB-4210-B
	DEVICE_DTS := gemini-nas4210b
	IMAGE/factory.bin := storlink-3072k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-3072k-sysupgrade-image $(1) |\
			append-metadata
	DEVICE_PACKAGES += $(GEMINI_NAS_PACKAGES)
endef
TARGET_DEVICES += raidsonic_ib-4210-b

define Device/raidsonic_ib-4220-b
	$(Device/storlink-reference)
	DEVICE_VENDOR := Raidsonic
	DEVICE_MODEL := NAS IB-4220-B
	DEVICE_DTS := gemini-nas4220b
	IMAGE/factory.bin := storlink-3072k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-3072k-sysupgrade-image $(1) |\
			append-metadata
	DEVICE_PACKAGES += $(GEMINI_NAS_PACKAGES)
endef
TARGET_DEVICES += raidsonic_ib-4220-b

define Device/storlink_sl93512r
	$(Device/storlink-reference)
	DEVICE_VENDOR := StorLink
	DEVICE_MODEL := SL93512r
	DEVICE_DTS := gemini-sl93512r
	IMAGE/factory.bin := storlink-3072k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-3072k-sysupgrade-image $(1) |\
			append-metadata
	DEVICE_PACKAGES += $(GEMINI_NAS_PACKAGES) kmod-dsa-vsc73xx-spi
endef
TARGET_DEVICES += storlink_sl93512r

define Device/teltonika_rut104
	$(Device/storlink-reference)
	# Only 32 MB of RAM and 8MB flash so not building by default
        DEFAULT := n
	DEVICE_VENDOR := Teltonika
	DEVICE_MODEL := RUT104
	DEVICE_DTS := gemini-rut1xx
	IMAGE/factory.bin := storlink-1024k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-1024k-sysupgrade-image $(1) |\
			append-metadata
	DEVICE_PACKAGES += kmod-phy-micrel kmod-ath5k
endef
TARGET_DEVICES += teltonika_rut104

define Device/verbatim_s08v1901_d1
	$(Device/storlink-reference)
	DEVICE_VENDOR := Verbatim
	DEVICE_MODEL := S08V1901-D1 (Gigabit Ethernet Hard Drive)
	DEVICE_DTS := gemini-verbatim-s08v1901-d1
	IMAGE/factory.bin := storlink-3072k-factory-image $(1)
	IMAGE/sysupgrade.bin := storlink-3072k-sysupgrade-image $(1) |\
			append-metadata
	DEVICE_PACKAGES += $(GEMINI_NAS_PACKAGES)
endef
TARGET_DEVICES += verbatim_s08v1901_d1

# The wiliboard images need some changes to be functional and buildable.
#
# The dts would need to use the redboot-fis partition parser to get
# the correct partition offsets and size.
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
