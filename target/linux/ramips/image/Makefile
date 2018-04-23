#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define imgname
$(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1)
endef

define sysupname
$(call imgname,$(1),$(2))-sysupgrade.bin
endef

VMLINUX:=$(IMG_PREFIX)-vmlinux
UIMAGE:=$(IMG_PREFIX)-uImage

define Image/Build/Initramfs
	$(call Image/Build/Profile/$(PROFILE),initramfs)
endef

DEVICE_VARS += DTS IMAGE_SIZE NETGEAR_BOARD_ID NETGEAR_HW_ID
DEVICE_VARS += BUFFALO_TAG_PLATFORM BUFFALO_TAG_VERSION BUFFALO_TAG_MINOR

loadaddr-y := 0x80000000
loadaddr-$(CONFIG_TARGET_ramips_rt288x) := 0x88000000
loadaddr-$(CONFIG_TARGET_ramips_mt7621) := 0x80001000

KERNEL_LOADADDR := $(loadaddr-y)

KERNEL_DTB = kernel-bin | patch-dtb | lzma
define Device/Default
  PROFILES = Default $$(DTS)
  KERNEL_DEPENDS = $$(wildcard ../dts/$$(DTS).dts)
  KERNEL := $(KERNEL_DTB) | uImage lzma
  DEVICE_DTS_DIR := ../dts
  IMAGES := sysupgrade.bin
  IMAGE_SIZE := $(ralink_default_fw_size_8M)
  SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
  sysupgrade_bin := append-kernel | append-rootfs | pad-rootfs
  IMAGE/sysupgrade.bin := append-kernel | append-rootfs | pad-rootfs | append-metadata | check-size $$$$(IMAGE_SIZE)
endef

define Build/patch-dtb
	$(call Image/BuildDTB,../dts/$(DTS).dts,$@.dtb)
	$(STAGING_DIR_HOST)/bin/patch-dtb $@ $@.dtb
endef

define Build/trx
	$(STAGING_DIR_HOST)/bin/trx $(1) \
		-o $@ \
		-m $(IMAGE_SIZE) \
		-f $(IMAGE_KERNEL) \
		-a 4 -f $(IMAGE_ROOTFS)
endef

define Build/relocate-kernel
	( \
		dd if=$(KDIR)/loader.bin bs=32 conv=sync && \
		perl -e '@s = stat("$@"); print pack("V", @s[7])' && \
		cat $@ \
	) > $@.new
	mv $@.new $@
endef

define MkCombineduImage
	$(call PatchKernelLzma,$(2),$(3))
	if [ `stat -c%s "$(KDIR)/vmlinux-$(2).bin.lzma"` -gt `expr $(4) - 64` ]; then \
		echo "Warning: $(KDIR)/vmlinux-$(2).bin.lzma is too big" >&2; \
	else if [ `stat -c%s "$(KDIR)/root.$(1)"` -gt $(5) ]; then \
		echo "Warning: $(KDIR)/root.$(1) is too big" >&2; \
	else \
		( dd if=$(KDIR)/vmlinux-$(2).bin.lzma bs=`expr $(4) - 64` conv=sync ; dd if=$(KDIR)/root.$(1) ) > $(KDIR)/vmlinux-$(2).bin.lzma.combined ; \
	fi ; fi
	$(call MkImage,lzma,$(KDIR)/vmlinux-$(2).bin.lzma.combined,$(call sysupname,$(1),$(2)),$(6))
endef

define Build/umedia-header
	fix-u-media-header -T 0x46 -B $(1) -i $@ -o $@.new && mv $@.new $@
endef

define Build/edimax-header
	$(STAGING_DIR_HOST)/bin/mkedimaximg -i $@ -o $@.new $(1)
	@mv $@.new $@
endef

define Build/poray-header
	$(STAGING_DIR_HOST)/bin/mkporayfw $(1) -f $@ -o $@.new
	mv $@.new $@
endef

define Build/senao-header
	$(STAGING_DIR_HOST)/bin/mksenaofw $(1) -e $@ -o $@.new
	mv $@.new $@
endef

define Build/seama
	$(STAGING_DIR_HOST)/bin/seama -i $@ $(1)
	mv $@.seama $@
endef

define Build/seama-seal
	$(call Build/seama,-s $@.seama $(1))
endef

define Build/wrg-header
	mkwrgimg -i $@ -d "/dev/mtdblock/2" -s $(1) -o $@.new
	mv $@.new $@
endef

# combine kernel and rootfs into one image
# mkdlinkfw <type> <optional extra arguments to mkdlinkfw binary>

define Build/mkdlinkfw
	-$(STAGING_DIR_HOST)/bin/mkdlinkfw \
		-k $(IMAGE_KERNEL) \
		-r $(IMAGE_ROOTFS) \
		-o $@ \
		-s $(DLINK_FIRMWARE_SIZE)
endef

define Build/mkdlinkfw-factory
	-$(STAGING_DIR_HOST)/bin/mkdlinkfw \
		-m $(DLINK_ROM_ID) -f $(DLINK_FAMILY_MEMBER) \
		-F $@ \
		-o $@.new \
		-s $(DLINK_FIRMWARE_SIZE)
	mv $@.new $@
endef

#
# The real magic happens inside these templates
#
# $(1), compression method
# $(2), filename of image data
# $(3), output filename
define MkImage
	$(eval imagename=$(if $(4),$(4),MIPS $(VERSION_DIST) Linux-$(LINUX_VERSION)))
	-mkimage -A mips -O linux -T kernel -C $(1) -a $(loadaddr-y) -e $(loadaddr-y) \
		-n "$(imagename)" \
		-d $(2) $(3)
endef

define CompressLzma
	$(STAGING_DIR_HOST)/bin/lzma e $(1) -lc1 -lp2 -pb2 $(2)
endef

define MkImageSysupgrade/squashfs
	$(eval output_name=$(IMG_PREFIX)-$(2)-$(1)-$(if $(4),$(4),sysupgrade).bin)
	cat $(KDIR)/vmlinux-$(2).uImage $(KDIR)/root.$(1) > $(KDIR)/$(output_name)
	$(call prepare_generic_squashfs,$(KDIR)/$(output_name))
	if [ `stat -c%s "$(KDIR)/$(output_name)"` -gt $(3) ]; then \
		echo "Warning: $(KDIR)/$(output_name) is too big" >&2; \
	else \
		$(CP) $(KDIR)/$(output_name) $(BIN_DIR)/$(output_name); \
	fi
endef

# $(1), lowercase board name like "mt7620a_v22sg"
# $(2), DTS filename without .dts extension
# $(3), optional filename suffix, e.g. "-initramfs"
define PatchKernelLzmaDtb
	cp $(KDIR)/vmlinux$(3) $(KDIR)/vmlinux-$(1)$(3)
	$(call Image/BuildDTB,../dts/$(2).dts,$(KDIR)/$(2).dtb)
	$(STAGING_DIR_HOST)/bin/patch-dtb $(KDIR)/vmlinux-$(1)$(3) $(KDIR)/$(2).dtb
	$(call CompressLzma,$(KDIR)/vmlinux-$(1)$(3),$(KDIR)/vmlinux-$(1)$(3).bin.lzma)
endef

# $(1), lowercase board name
# $(2), DTS filename without .dts extension
# $(3), ih_name field of uImage header
# $(4), optional filename suffix, e.g. "-initramfs"
define MkImageLzmaDtb
	$(call PatchKernelLzmaDtb,$(1),$(2),$(4))
	$(call MkImage,lzma,$(KDIR)/vmlinux-$(1)$(4).bin.lzma,$(KDIR)/vmlinux-$(1)$(4).uImage,$(3))
endef

# $(1), Rootfs type, e.g. squashfs
# $(2), lowercase board name
# $(3), DTS filename without .dts extension
# $(4), maximum size of sysupgrade image
# $(5), uImage header's ih_name field
define BuildFirmware/OF
	$(call MkImageLzmaDtb,$(2),$(3),$(5))
	$(call MkImageSysupgrade/$(1),$(1),$(2),$(4),$(6))
endef

# $(1), squashfs/initramfs
# $(2), lowercase board name
# $(3), DTS filename without .dts extension
# $(4), ih_name field of uImage header
define BuildFirmware/OF/initramfs
	$(call MkImageLzmaDtb,$(2),$(3),$(4),-initramfs)
	$(CP) $(KDIR)/vmlinux-$(2)-initramfs.uImage $(call imgname,$(1),$(2))-uImage.bin
endef

# Build images for default ralink layout for 4MB flash
# kernel + roots = 0x3b0000
# $(1) = squashfs/initramfs
# $(2) = lowercase board name
# $(3) = dts file
ralink_default_fw_size_4M=3866624
BuildFirmware/Default4M/squashfs=$(call BuildFirmware/OF,$(1),$(2),$(3),$(ralink_default_fw_size_4M),$(4))
BuildFirmware/Default4M/initramfs=$(call BuildFirmware/OF/initramfs,$(1),$(2),$(3),$(4))

ralink_default_fw_size_8M=8060928
ralink_default_fw_size_16M=16121856
ralink_default_fw_size_32M=33226752


ifeq ($(SUBTARGET),rt288x)
include rt288x.mk
endif

ifeq ($(SUBTARGET),rt305x)
include rt305x.mk
include rt305x-legacy.mk
endif

ifeq ($(SUBTARGET),rt3883)
include rt3883.mk
endif

ifeq ($(SUBTARGET),mt7620)
include mt7620.mk
endif

ifeq ($(SUBTARGET),mt7621)
include mt7621.mk
endif

ifeq ($(SUBTARGET),mt76x8)
include mt76x8.mk
endif

ifdef LEGACY_DEVICES
#
# Generic Targets - only needed for legacy image building code
#
define Image/BuildKernel
	cp $(KDIR)/vmlinux.elf $(BIN_DIR)/$(VMLINUX).elf
	cp $(KDIR)/vmlinux $(BIN_DIR)/$(VMLINUX).bin
	$(call CompressLzma,$(KDIR)/vmlinux,$(KDIR)/vmlinux.bin.lzma)
	$(call MkImage,lzma,$(KDIR)/vmlinux.bin.lzma,$(KDIR)/uImage.lzma)
	cp $(KDIR)/uImage.lzma $(BIN_DIR)/$(UIMAGE).bin
endef

define Image/BuildKernel/Initramfs
	cp $(KDIR)/vmlinux-initramfs.elf $(BIN_DIR)/$(VMLINUX)-initramfs.elf
	cp $(KDIR)/vmlinux-initramfs $(BIN_DIR)/$(VMLINUX)-initramfs.bin
	$(call CompressLzma,$(KDIR)/vmlinux-initramfs,$(KDIR)/vmlinux-initramfs.bin.lzma)
	$(call MkImage,lzma,$(KDIR)/vmlinux-initramfs.bin.lzma,$(KDIR)/uImage-initramfs.lzma)
	cp $(KDIR)/uImage-initramfs.lzma $(BIN_DIR)/$(UIMAGE)-initramfs.bin
	$(call Image/Build/Initramfs)
endef

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
endef

define Image/Build/Profile
	$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS),$(call Image/Build/Profile/$(1),initramfs))
	$(call Image/Build/Profile/$(1),$(2))
endef

endif

$(eval $(call BuildImage))
