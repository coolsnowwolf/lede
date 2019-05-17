#
# Copyright (C) 2006-2015 OpenWrt.org
# Copyright (C) 2016 LEDE project
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

LOADADDR = 0x80010000		# RAM start + 64K
KERNEL_ENTRY = $(LOADADDR)	# Newer kernels add a jmp to the kernel_entry at the start of the binary
LOADER_ENTRY = 0x80a00000	# RAM start + 10M, for relocate
RAMSIZE = 0x02000000		# 32MB
LZMA_TEXT_START = 0x81800000	# 32MB - 8MB

LOADER_MAKEOPTS= \
		KDIR=$(KDIR) \
		LOADADDR=$(LOADADDR) \
		KERNEL_ENTRY=$(KERNEL_ENTRY) \
		RAMSIZE=$(RAMSIZE) \
		LZMA_TEXT_START=$(LZMA_TEXT_START) \

RELOCATE_MAKEOPTS= \
		CACHELINE_SIZE=16 \
		KERNEL_ADDR=$(KERNEL_ENTRY) \
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
		--output_file=$@.hcs --ldaddress=$(LOADADDR)
	mv $@.hcs $@
endef

define Build/loader-lzma
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		$(LOADER_MAKEOPTS) \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" \
		LOADER_DATA="$@" \
		LOADER_NAME="$(notdir $@)" \
		compile loader.$(1)
	mv "$@.$(1)" "$@"
	rm -rf $@.src
endef

define Build/lzma
	# CFE is a LZMA nazi! It took me hours to find out the parameters!
	# Also I think lzma has a bug cause it generates different output depending on
	# if you use stdin / stdout or not. Use files instead of stdio here, cause
	# otherwise CFE will complain and not boot the image.
	$(STAGING_DIR_HOST)/bin/lzma e $@ -d22 -fb64 -a1 $@.lzma
	mv $@.lzma $@
endef

define Build/lzma-cfe
	# Strip out the length, CFE doesn't like this
	dd if=$@ of=$@.lzma.cfe bs=5 count=1
	dd if=$@ of=$@.lzma.cfe ibs=13 obs=5 skip=1 seek=1 conv=notrunc
	mv $@.lzma.cfe $@
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
		--output $@ --boardid $(CFE_BOARD_ID) --chipid $(CFE_CHIP_ID) \
		--entry $(LOADER_ENTRY) --load-addr $(LOADER_ENTRY) \
		--info1 "$(call Image/LimitName16,$(DEVICE_NAME))" \
		--info2 "$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))" \
		$(call rootfspad/$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))) \
		$(CFE_EXTRAS) $(1)
endef

define Build/cfe-old-bin
	$(TOPDIR)/scripts/brcmImage.pl -t -p \
		-o $@ -b $(CFE_BOARD_ID) -c $(CFE_CHIP_ID) \
		-e $(LOADER_ENTRY) -a $(LOADER_ENTRY) \
		-k $(IMAGE_KERNEL) -r $(IMAGE_ROOTFS) \
		$(CFE_EXTRAS)
endef

define Build/cfe-spw303v-bin
	$(STAGING_DIR_HOST)/bin/imagetag -i $(IMAGE_KERNEL) -f $(IMAGE_ROOTFS) \
		--output $@ --boardid $(CFE_BOARD_ID) --chipid $(CFE_CHIP_ID) \
		--entry $(LOADER_ENTRY) --load-addr $(LOADER_ENTRY) \
		$(call rootfspad/$(call Image/FileSystemStrip,$(IMAGE_ROOTFS))) \
		$(CFE_EXTRAS) $(1)
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
endef

ATH5K_PACKAGES := kmod-ath5k wpad-basic
ATH9K_PACKAGES := kmod-ath9k wpad-basic
B43_PACKAGES := kmod-b43 wpad-basic
BRCMWL_PACKAGES := kmod-brcm-wl nas wlc
RT28_PACKAGES := kmod-rt2800-pci wpad-basic
RT61_PACKAGES := kmod-rt61-pci wpad-basic
USB1_PACKAGES := kmod-usb-ohci kmod-usb-ledtrig-usbport
USB2_PACKAGES := kmod-usb2 kmod-usb-ohci kmod-usb-ledtrig-usbport

include bcm63xx.mk

$(eval $(call BuildImage))
