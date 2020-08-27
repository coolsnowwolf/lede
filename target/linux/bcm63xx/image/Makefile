#
# Copyright (C) 2006-2015 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80010000		# RAM start + 64K
LOADER_ENTRY = 0x80a00000		# RAM start + 10M, for relocate
RAMSIZE = 0x02000000			# 32MB
LZMA_TEXT_START = 0x81800000		# 32MB - 8MB

RELOCATE_MAKEOPTS= \
		CACHELINE_SIZE=16 \
		KERNEL_ADDR=$(KERNEL_LOADADDR) \
		CROSS_COMPILE=$(TARGET_CROSS) \
		LZMA_TEXT_START=$(LOADER_ENTRY)

define Build/Compile
	rm -rf $(KDIR)/relocate
	$(CP) ../../generic/image/relocate $(KDIR)
	$(MAKE) -C $(KDIR)/relocate $(RELOCATE_MAKEOPTS)
endef

### Kernel scripts ###
define Build/hcs-initramfs
	$(STAGING_DIR_HOST)/bin/hcsmakeimage --magic_bytes=$(HCS_MAGIC_BYTES) \
		--rev_maj=$(HCS_REV_MAJ) --rev_min=$(HCS_REV_MIN) --input_file=$@ \
		--output_file=$@.hcs --ldaddress=$(KERNEL_LOADADDR)
	mv $@.hcs $@
endef

define Build/loader-lzma
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		KDIR=$(KDIR) \
		LOADER_ADDR=$(if $(DEVICE_LOADADDR),$(DEVICE_LOADADDR),$(LOADER_ENTRY)) \
		KERNEL_ADDR=$(KERNEL_LOADADDR) \
		RAMSIZE=$(RAMSIZE) \
		LZMA_TEXT_START=$(LZMA_TEXT_START) \
		CHIP_ID=$(CHIP_ID) \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" \
		LOADER_DATA="$@" \
		LOADER_NAME="$(notdir $@)" \
		compile loader.$(1)
	mv "$@.$(1)" "$@"
	rm -rf $@.src
endef

define Build/lzma-cfe
	# CFE is a LZMA nazi! It took me hours to find out the parameters!
	# Also I think lzma has a bug cause it generates different output depending on
	# if you use stdin / stdout or not. Use files instead of stdio here, cause
	# otherwise CFE will complain and not boot the image.
	$(call Build/lzma-no-dict,-d22 -fb64 -a1)
	# Strip out the length, CFE doesn't like this
	dd if=$@ of=$@.new bs=5 count=1
	dd if=$@ of=$@.new ibs=13 obs=5 skip=1 seek=1 conv=notrunc
	mv $@.new $@
endef

define Build/relocate-kernel
	# CFE only allows ~4 MiB for the uncompressed kernels, but uncompressed
	# kernel might get larger than that, so let CFE unpack and load at a
	# higher address and make the kernel relocate itself to the expected
	# location.
	( \
		dd if=$(KDIR)/relocate/loader.bin bs=32 conv=sync && \
		perl -e '@s = stat("$@"); print pack("N", @s[7])' && \
		cat $@ \
	) > $@.relocate
	mv $@.relocate $@
endef

### Image scripts ###
define rootfspad/jffs2-128k
--align-rootfs
endef
define rootfspad/jffs2-64k
--align-rootfs
endef
define rootfspad/squashfs
endef

define Image/LimitName16
$(shell expr substr "$(1)" 1 16)
endef

define Image/FileSystemStrip
$(firstword $(subst +,$(space),$(subst root.,,$(notdir $(1)))))
endef

define Build/cfe-bin
	$(STAGING_DIR_HOST)/bin/imagetag -i $(IMAGE_KERNEL) -f $(IMAGE_ROOTFS) \
		--output $@ --boardid $(CFE_BOARD_ID) --chipid $(CHIP_ID) \
		--entry $(LOADER_ENTRY) --load-addr $(LOADER_ENTRY) \
		--info1 "$(call Image/LimitName16,$(DEVICE_NAME))" \
		--info2 "$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))" \
		$(call rootfspad/$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))) \
		$(CFE_EXTRAS) $(1)
endef

define Build/cfe-jffs2
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		--big-endian \
		--pad \
		--no-cleanmarkers \
		--eraseblock=$(patsubst %k,%KiB,$(BLOCKSIZE)) \
		--root=$(1) \
		--output=$@ \
		--compression-mode=none

	$(call Build/pad-to,$(BLOCKSIZE))
endef

define Build/cfe-jffs2-cferam
	mv $@ $@.kernel

	rm -rf $@-cferam
	mkdir -p $@-cferam

	# CFE ROM checks JFFS2 dirent version of cferam.
	# If version is not > 0 it will ignore the fs entry.
	# JFFS2 sets version 0 to the first fs entry and increments
	# it on the following ones, so let's create a dummy file that
	# will have version 0 and let cferam be the second (version 1).
	touch $@-cferam/1-openwrt
	# Add cferam as the last file in the JFFS2 partition
	cp $(KDIR)/bcm63xx-cfe/$(CFE_RAM_FILE) $@-cferam/$(CFE_RAM_JFFS2_NAME)

	# The JFFS2 partition creation should result in the following
	# layout:
	# 1) 1-openwrt (version 0, ino 2)
	# 2) cferam.000 (version 1, ino 3)
	$(call Build/cfe-jffs2,$@-cferam)

	# Some devices need padding between CFE RAM and kernel
	$(if $(CFE_RAM_JFFS2_PAD),$(call Build/pad-to,$(CFE_RAM_JFFS2_PAD)))

	# Add CFE partition tag
	$(if $(CFE_PART_ID),$(call Build/cfe-part-tag))

	# Append kernel
	dd if=$@.kernel >> $@
	rm -f $@.kernel
endef

define Build/cfe-jffs2-kernel
	rm -rf $@-kernel
	mkdir -p $@-kernel

	# CFE RAM checks JFFS2 dirent version of vmlinux.
	# If version is not > 0 it will ignore the fs entry.
	# JFFS2 sets version 0 to the first fs entry and increments
	# it on the following ones, so let's create a dummy file that
	# will have version 0 and let cferam be the second (version 1).
	touch $@-kernel/1-openwrt
	# vmlinux is located on a different JFFS2 partition, but CFE RAM
	# ignores it, so let's create another dummy file that will match
	# the JFFS2 ino of cferam entry on the first JFFS2 partition.
	# CFE RAM won't be able to find vmlinux if cferam has the same
	# ino as vmlinux.
	touch $@-kernel/2-openwrt
	# Add vmlinux as the last file in the JFFS2 partition
	$(TOPDIR)/scripts/cfe-bin-header.py \
		--input-file $@ \
		--output-file $@-kernel/vmlinux.lz \
		--load-addr $(if $(DEVICE_LOADADDR),$(DEVICE_LOADADDR),$(LOADER_ENTRY)) \
		--entry-addr $(if $(DEVICE_LOADADDR),$(DEVICE_LOADADDR),$(LOADER_ENTRY))

	# The JFFS2 partition creation should result in the following
	# layout:
	# 1) 1-openwrt (version 0, ino 2)
	# 2) 2-openwrt (version 1, ino 3)
	# 3) vmlinux.lz (version 2, ino 4)
	$(call Build/cfe-jffs2,$@-kernel)
endef

define Build/cfe-part-tag
	mv $@ $@.part

	$(TOPDIR)/scripts/cfe-partition-tag.py \
		--input-file $@.part \
		--output-file $@ \
		--flags $(CFE_PART_FLAGS) \
		--id $(CFE_PART_ID) \
		--name $(VERSION_CODE) \
		--version $(DEVICE_NAME)

	$(call Build/pad-to,$(BLOCKSIZE))

	dd if=$@.part >> $@
endef

define Build/cfe-sercomm-part
	$(TOPDIR)/scripts/sercomm-partition-tag.py \
		--input-file $@ \
		--output-file $@.kernel_rootfs \
		--part-name kernel_rootfs \
		--part-version OpenWrt \
		--rootfs-version $(SERCOMM_VERSION)

	rm -rf $@-rootfs_lib
	mkdir -p $@-rootfs_lib
	echo $(SERCOMM_VERSION) > $@-rootfs_lib/lib_ver
	$(call Build/cfe-jffs2,$@-rootfs_lib)
	$(call Build/pad-to,$(BLOCKSIZE))
	$(TOPDIR)/scripts/sercomm-partition-tag.py \
		--input-file $@ \
		--output-file $@.rootfs_lib \
		--part-name rootfs_lib \
		--part-version $(SERCOMM_VERSION)

	mv $@.kernel_rootfs $@
	dd if=$@.rootfs_lib >> $@
endef

define Build/cfe-sercomm-load
	$(TOPDIR)/scripts/sercomm-payload.py \
		--input-file $@ \
		--output-file $@.new \
		--pid "$(SERCOMM_PID)"

	mv $@.new $@
endef

define Build/cfe-sercomm-crypto
	$(TOPDIR)/scripts/sercomm-crypto.py \
		--input-file $@ \
		--key-file $@.key \
		--output-file $@.ser \
		--version OpenWrt
	$(STAGING_DIR_HOST)/bin/openssl enc -md md5 -aes-256-cbc \
		-in $@ -out $@.enc \
		-K `cat $@.key` \
		-iv 00000000000000000000000000000000
	dd if=$@.enc >> $@.ser
	mv $@.ser $@
	rm -f $@.enc $@.key
endef

define Build/cfe-old-bin
	$(TOPDIR)/scripts/brcmImage.pl -t -p \
		-o $@ -b $(CFE_BOARD_ID) -c $(CHIP_ID) \
		-e $(LOADER_ENTRY) -a $(LOADER_ENTRY) \
		-k $(IMAGE_KERNEL) -r $(IMAGE_ROOTFS) \
		$(CFE_EXTRAS)
endef

define Build/cfe-spw303v-bin
	$(STAGING_DIR_HOST)/bin/imagetag -i $(IMAGE_KERNEL) -f $(IMAGE_ROOTFS) \
		--output $@ --boardid $(CFE_BOARD_ID) --chipid $(CHIP_ID) \
		--entry $(LOADER_ENTRY) --load-addr $(LOADER_ENTRY) \
		$(call rootfspad/$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))) \
		$(CFE_EXTRAS) $(1)
endef

define Build/cfe-wfi-tag
	$(TOPDIR)/scripts/cfe-wfi-tag.py \
		--input-file $@ \
		--output-file $@.new \
		--version $(if $(1),$(1),$(CFE_WFI_VERSION)) \
		--chip-id $(CFE_WFI_CHIP_ID) \
		--flash-type $(CFE_WFI_FLASH_TYPE) \
		$(if $(CFE_WFI_FLAGS),--flags $(CFE_WFI_FLAGS))
	mv $@.new $@
endef

define Build/spw303v-bin
	$(STAGING_DIR_HOST)/bin/spw303v -i $@ -o $@.spw303v
	mv $@.spw303v $@
endef

define Build/zyxel-bin
	$(STAGING_DIR_HOST)/bin/zyxbcm -i $@ -o $@.zyxel
	mv $@.zyxel $@
endef

define Build/redboot-bin
	# Prepare kernel and rootfs
	dd if=$(IMAGE_KERNEL) of=$(BIN_DIR)/$(REDBOOT_PREFIX)-vmlinux.gz bs=65536 conv=sync
	dd if=$(IMAGE_ROOTFS) of=$(BIN_DIR)/$(REDBOOT_PREFIX)-$(notdir $(IMAGE_ROOTFS)) bs=64k conv=sync
	echo -ne \\xDE\\xAD\\xC0\\xDE >> $(BIN_DIR)/$(REDBOOT_PREFIX)-$(notdir $(IMAGE_ROOTFS))
	# Generate the scripted image
	$(TOPDIR)/scripts/redboot-script.pl \
		-k $(BIN_DIR)/$(REDBOOT_PREFIX)-vmlinux.gz \
		-r $(BIN_DIR)/$(REDBOOT_PREFIX)-$(notdir $(IMAGE_ROOTFS)) \
		-a $(strip $(LOADER_ENTRY)) -f 0xbe430000 -l 0x7c0000 \
		-s 0x1000 -t 20 -o $@.redbootscript
	dd if="$@.redbootscript" of="$@.redbootscript.padded" bs=4096 conv=sync
	cat \
		"$@.redbootscript.padded" \
		"$(BIN_DIR)/$(REDBOOT_PREFIX)-vmlinux.gz" \
		"$(BIN_DIR)/$(REDBOOT_PREFIX)-$(notdir $(IMAGE_ROOTFS))" \
		> "$@"
endef

define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  KERNEL_DEPENDS = $$(wildcard ../dts/$$(DEVICE_DTS).dts)
  KERNEL_INITRAMFS_SUFFIX := .elf
  DEVICE_DTS_DIR := ../dts
  CHIP_ID :=
  SOC = bcm$$(CHIP_ID)
  DEVICE_DTS = $$(SOC)-$(subst _,-,$(1))
  DEVICE_LOADADDR :=
endef
DEVICE_VARS += CHIP_ID DEVICE_LOADADDR

ATH5K_PACKAGES := kmod-ath5k wpad-basic-wolfssl
ATH9K_PACKAGES := kmod-ath9k wpad-basic-wolfssl
B43_PACKAGES := kmod-b43 wpad-basic-wolfssl
BRCMWL_PACKAGES := kmod-brcm-wl nas wlc
RT28_PACKAGES := kmod-rt2800-pci wpad-basic-wolfssl
RT61_PACKAGES := kmod-rt61-pci wpad-basic-wolfssl
USB1_PACKAGES := kmod-usb-ohci kmod-usb-ledtrig-usbport
USB2_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport

include bcm63xx.mk

ifeq ($(SUBTARGET),smp)
include bcm63xx_nand.mk
endif

$(eval $(call BuildImage))
