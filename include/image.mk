# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2020 OpenWrt.org

override TARGET_BUILD=
include $(INCLUDE_DIR)/prereq.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk
include $(INCLUDE_DIR)/image-commands.mk

ifndef IB
  ifdef CONFIG_TARGET_PER_DEVICE_ROOTFS
    TARGET_PER_DEVICE_ROOTFS := 1
  endif
endif

include $(INCLUDE_DIR)/feeds.mk
include $(INCLUDE_DIR)/rootfs.mk

override MAKE:=$(_SINGLE)$(SUBMAKE)
override NO_TRACE_MAKE:=$(_SINGLE)$(NO_TRACE_MAKE)

target_params = $(subst +,$(space),$*)
param_get = $(patsubst $(1)=%,%,$(filter $(1)=%,$(2)))
param_get_default = $(firstword $(call param_get,$(1),$(2)) $(3))
param_mangle = $(subst $(space),_,$(strip $(1)))
param_unmangle = $(subst _,$(space),$(1))

mkfs_packages_id = $(shell echo $(sort $(1)) | $(MKHASH) md5 | cut -b1-8)
mkfs_target_dir = $(if $(call param_get,pkg,$(1)),$(KDIR)/target-dir-$(call param_get,pkg,$(1)),$(TARGET_DIR))

KDIR=$(KERNEL_BUILD_DIR)
KDIR_TMP=$(KDIR)/tmp
DTS_DIR:=$(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts

IMG_PREFIX_EXTRA:=$(if $(EXTRA_IMAGE_NAME),$(call sanitize,$(EXTRA_IMAGE_NAME))-)
IMG_PREFIX_VERNUM:=$(if $(CONFIG_VERSION_FILENAMES),$(call sanitize,$(VERSION_NUMBER))-)
IMG_PREFIX_VERCODE:=$(if $(CONFIG_VERSION_CODE_FILENAMES),$(call sanitize,$(VERSION_CODE))-)

IMG_PREFIX:=$(VERSION_DIST_SANITIZED)-$(IMG_PREFIX_VERNUM)$(IMG_PREFIX_VERCODE)$(IMG_PREFIX_EXTRA)$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))
IMG_ROOTFS:=$(IMG_PREFIX)-rootfs
IMG_COMBINED:=$(IMG_PREFIX)-combined
IMG_PART_SIGNATURE:=$(shell echo $(SOURCE_DATE_EPOCH)$(LINUX_VERMAGIC) | $(MKHASH) md5 | cut -b1-8)
IMG_PART_DISKGUID:=$(shell echo $(SOURCE_DATE_EPOCH)$(LINUX_VERMAGIC) | $(MKHASH) md5 | sed -E 's/(.{8})(.{4})(.{4})(.{4})(.{10})../\1-\2-\3-\4-\500/')

MKFS_DEVTABLE_OPT := -D $(INCLUDE_DIR)/device_table.txt

ifneq ($(CONFIG_BIG_ENDIAN),)
  JFFS2OPTS     :=  --big-endian --squash-uids -v
else
  JFFS2OPTS     :=  --little-endian --squash-uids -v
endif

ifeq ($(CONFIG_JFFS2_RTIME),y)
  JFFS2OPTS += -X rtime
endif
ifeq ($(CONFIG_JFFS2_ZLIB),y)
  JFFS2OPTS += -X zlib
endif
ifeq ($(CONFIG_JFFS2_LZMA),y)
  JFFS2OPTS += -X lzma --compression-mode=size
endif
ifneq ($(CONFIG_JFFS2_RTIME),y)
  JFFS2OPTS += -x rtime
endif
ifneq ($(CONFIG_JFFS2_ZLIB),y)
  JFFS2OPTS += -x zlib
endif
ifneq ($(CONFIG_JFFS2_LZMA),y)
  JFFS2OPTS += -x lzma
endif

JFFS2OPTS += $(MKFS_DEVTABLE_OPT)

SQUASHFS_BLOCKSIZE := $(CONFIG_TARGET_SQUASHFS_BLOCK_SIZE)k
SQUASHFSOPT := -b $(SQUASHFS_BLOCKSIZE)
SQUASHFSOPT += -p '/dev d 755 0 0' -p '/dev/console c 600 0 0 5 1'
SQUASHFSOPT += $(if $(CONFIG_SELINUX),-xattrs,-no-xattrs)
SQUASHFSCOMP := gzip
LZMA_XZ_OPTIONS := -Xpreset 9 -Xe -Xlc 0 -Xlp 2 -Xpb 2
ifeq ($(CONFIG_SQUASHFS_XZ),y)
  ifneq ($(filter arm x86 powerpc sparc,$(LINUX_KARCH)),)
    BCJ_FILTER:=-Xbcj $(LINUX_KARCH)
  endif
  SQUASHFSCOMP := xz $(LZMA_XZ_OPTIONS) $(BCJ_FILTER)
endif

JFFS2_BLOCKSIZE ?= 64k 128k

fs-types-$(CONFIG_TARGET_ROOTFS_SQUASHFS) += squashfs
fs-types-$(CONFIG_TARGET_ROOTFS_JFFS2) += $(addprefix jffs2-,$(JFFS2_BLOCKSIZE))
fs-types-$(CONFIG_TARGET_ROOTFS_JFFS2_NAND) += $(addprefix jffs2-nand-,$(NAND_BLOCKSIZE))
fs-types-$(CONFIG_TARGET_ROOTFS_EXT4FS) += ext4
fs-types-$(CONFIG_TARGET_ROOTFS_UBIFS) += ubifs
fs-subtypes-$(CONFIG_TARGET_ROOTFS_JFFS2) += $(addsuffix -raw,$(addprefix jffs2-,$(JFFS2_BLOCKSIZE)))

TARGET_FILESYSTEMS := $(fs-types-y)

FS_64K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-64k
FS_128K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-128k
FS_256K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-256k

define add_jffs2_mark
	echo -ne '\xde\xad\xc0\xde' >> $(1)
endef

PROFILE_SANITIZED := $(call tolower,$(subst DEVICE_,,$(subst $(space),-,$(PROFILE))))

define split_args
$(foreach data, \
	$(subst |,$(space),\
		$(subst $(space),^,$(1))), \
	$(call $(2),$(strip $(subst ^,$(space),$(data)))))
endef

define build_cmd
$(if $(Build/$(word 1,$(1))),,$(error Missing Build/$(word 1,$(1))))
$(call Build/$(word 1,$(1)),$(wordlist 2,$(words $(1)),$(1)))

endef

define concat_cmd
$(call split_args,$(1),build_cmd)
endef

# pad to 4k, 8k, 16k, 64k, 128k, 256k and add jffs2 end-of-filesystem mark
define prepare_generic_squashfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(1) 4 8 16 64 128 256
endef

define Image/BuildKernel/Initramfs
	$(call Image/Build/Initramfs)
endef

define Image/BuildKernel/MkuImage
	mkimage -A $(ARCH) -O linux -T kernel -C $(1) -a $(2) -e $(3) \
		-n '$(call toupper,$(ARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION)' -d $(4) $(5)
endef

define Image/BuildKernel/MkFIT
	$(TOPDIR)/scripts/mkits.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(if $(3),-d $(3)) -C $(4) -a $(5) -e $(6) \
		-c $(if $(DEVICE_DTS_CONFIG),$(DEVICE_DTS_CONFIG),"config-1") \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1)$(7).itb
endef

ifdef CONFIG_TARGET_IMAGES_GZIP
  define Image/Gzip
	rm -f $(1).gz
	gzip -9n $(1)
  endef
endif


# Disable noisy checks by default as in upstream
DTC_FLAGS += \
  -Wno-unit_address_vs_reg \
  -Wno-simple_bus_reg \
  -Wno-unit_address_format \
  -Wno-pci_bridge \
  -Wno-pci_device_bus_num \
  -Wno-pci_device_reg \
  -Wno-avoid_unnecessary_addr_size \
  -Wno-alias_paths \
  -Wno-graph_child_address \
  -Wno-graph_port \
  -Wno-unique_unit_address

define Image/pad-to
	dd if=$(1) of=$(1).new bs=$(2) conv=sync
	mv $(1).new $(1)
endef

ROOTFS_PARTSIZE=$(shell echo $$(($(CONFIG_TARGET_ROOTFS_PARTSIZE)*1024*1024)))

define Image/pad-root-squashfs
	$(call Image/pad-to,$(KDIR)/root.squashfs,$(if $(1),$(1),$(ROOTFS_PARTSIZE)))
endef

# $(1) source dts file
# $(2) target dtb file
# $(3) extra CPP flags
# $(4) extra DTC flags
define Image/BuildDTB
	$(TARGET_CROSS)cpp -nostdinc -x assembler-with-cpp \
		-I$(DTS_DIR) \
		-I$(DTS_DIR)/include \
		-I$(LINUX_DIR)/include/ \
		-undef -D__DTS__ $(3) \
		-o $(2).tmp $(1)
	$(LINUX_DIR)/scripts/dtc/dtc -O dtb \
		-i$(dir $(1)) $(DTC_FLAGS) $(4) \
	$(if $(CONFIG_HAS_DT_OVERLAY_SUPPORT),-@) \
		-o $(2) $(2).tmp
	$(RM) $(2).tmp
endef

define Image/mkfs/jffs2/sub-raw
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		$(2) \
		-e $(patsubst %k,%KiB,$(1)) \
		-o $@ -d $(call mkfs_target_dir,$(3)) \
		-v 2>&1 1>/dev/null | awk '/^.+$$$$/'
endef

define Image/mkfs/jffs2/sub
	$(call Image/mkfs/jffs2/sub-raw,$(1),--pad $(2),$(3))
	$(call add_jffs2_mark,$@)
endef

define Image/mkfs/jffs2/template
  Image/mkfs/jffs2-$(1) = $$(call Image/mkfs/jffs2/sub,$(1),$(JFFS2OPTS),$$(1))
  Image/mkfs/jffs2-$(1)-raw = $$(call Image/mkfs/jffs2/sub-raw,$(1),$(JFFS2OPTS),$$(1))

endef

define Image/mkfs/jffs2-nand/template
  Image/mkfs/jffs2-nand-$(1) = \
	$$(call Image/mkfs/jffs2/sub, \
		$(word 2,$(subst -, ,$(1))), \
			$(JFFS2OPTS) --no-cleanmarkers --pagesize=$(word 1,$(subst -, ,$(1))),$$(1))

endef

$(eval $(foreach S,$(JFFS2_BLOCKSIZE),$(call Image/mkfs/jffs2/template,$(S))))
$(eval $(foreach S,$(NAND_BLOCKSIZE),$(call Image/mkfs/jffs2-nand/template,$(S))))

define Image/mkfs/squashfs-common
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(call mkfs_target_dir,$(1)) $@ \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT) \
		-processors $(shell nproc)
endef

ifeq ($(CONFIG_TARGET_ROOTFS_SECURITY_LABELS),y)
define Image/mkfs/squashfs
	echo ". $(call mkfs_target_dir,$(1))/etc/selinux/config" > $@.fakeroot-script
	echo "$(STAGING_DIR_HOST)/bin/setfiles -r" \
	     "$(call mkfs_target_dir,$(1))" \
	     "$(call mkfs_target_dir,$(1))/etc/selinux/\$${SELINUXTYPE}/contexts/files/file_contexts " \
	     "$(call mkfs_target_dir,$(1))" >> $@.fakeroot-script
	echo "$(Image/mkfs/squashfs-common)" >> $@.fakeroot-script
	chmod +x $@.fakeroot-script
	$(FAKEROOT) "$@.fakeroot-script"
endef
else
define Image/mkfs/squashfs
	$(call Image/mkfs/squashfs-common,$(1))
endef
endif

define Image/mkfs/ubifs
	$(STAGING_DIR_HOST)/bin/mkfs.ubifs \
		$(UBIFS_OPTS) $(call param_unmangle,$(call param_get,fs,$(1))) \
		$(if $(CONFIG_TARGET_UBIFS_FREE_SPACE_FIXUP),--space-fixup) \
		$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_NONE),--compr=none) \
		$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_LZO),--compr=lzo) \
		$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_ZLIB),--compr=zlib) \
		$(if $(shell echo $(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)),--jrn-size=$(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)) \
		--squash-uids \
		-o $@ -d $(call mkfs_target_dir,$(1))
endef

define Image/mkfs/ext4
	$(STAGING_DIR_HOST)/bin/make_ext4fs -L rootfs \
		-l $(ROOTFS_PARTSIZE) -b $(CONFIG_TARGET_EXT4_BLOCKSIZE) \
		$(if $(CONFIG_TARGET_EXT4_RESERVED_PCT),-m $(CONFIG_TARGET_EXT4_RESERVED_PCT)) \
		$(if $(CONFIG_TARGET_EXT4_JOURNAL),,-J) \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		$@ $(call mkfs_target_dir,$(1))/
endef

define Image/Manifest
	$(call opkg,$(TARGET_DIR_ORIG)) list-installed > \
		$(BIN_DIR)/$(IMG_PREFIX)$(if $(PROFILE_SANITIZED),-$(PROFILE_SANITIZED)).manifest
endef

define Image/gzip-ext4-padded-squashfs

  define Image/Build/squashfs
    $(call Image/pad-root-squashfs)
  endef

  ifneq ($(CONFIG_TARGET_IMAGES_GZIP),)
    define Image/Build/gzip/ext4
      $(call Image/Build/gzip,ext4)
    endef
    define Image/Build/gzip/squashfs
      $(call Image/Build/gzip,squashfs)
    endef
  endif

endef

ifdef CONFIG_TARGET_ROOTFS_TARGZ
  define Image/Build/targz
	$(TAR) -cp --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-C $(TARGET_DIR)/ . | gzip -9n > $(BIN_DIR)/$(IMG_PREFIX)$(if $(PROFILE_SANITIZED),-$(PROFILE_SANITIZED))-rootfs.tar.gz
  endef
endif

ifdef CONFIG_TARGET_ROOTFS_CPIOGZ
  define Image/Build/cpiogz
	( cd $(TARGET_DIR); find . | $(STAGING_DIR_HOST)/bin/cpio -o -H newc -R 0:0 | gzip -9n >$(BIN_DIR)/$(IMG_ROOTFS).cpio.gz )
  endef
endif

mkfs_packages = $(filter-out @%,$(PACKAGES_$(call param_get,pkg,pkg=$(target_params))))
mkfs_packages_add = $(foreach pkg,$(filter-out -%,$(mkfs_packages)),$(pkg)$(call GetABISuffix,$(pkg)))
mkfs_packages_remove = $(foreach pkg,$(patsubst -%,%,$(filter -%,$(mkfs_packages))),$(pkg)$(call GetABISuffix,$(pkg)))
mkfs_cur_target_dir = $(call mkfs_target_dir,pkg=$(target_params))

opkg_target = \
	$(call opkg,$(mkfs_cur_target_dir)) \
		-f $(mkfs_cur_target_dir).conf

target-dir-%: FORCE
	rm -rf $(mkfs_cur_target_dir) $(mkfs_cur_target_dir).opkg
	$(CP) $(TARGET_DIR_ORIG) $(mkfs_cur_target_dir)
	-mv $(mkfs_cur_target_dir)/etc/opkg $(mkfs_cur_target_dir).opkg
	echo 'src default file://$(PACKAGE_DIR_ALL)' > $(mkfs_cur_target_dir).conf
	$(if $(mkfs_packages_remove), \
		-$(call opkg,$(mkfs_cur_target_dir)) remove \
			$(mkfs_packages_remove))
	$(if $(call opkg_package_files,$(mkfs_packages_add)), \
		$(opkg_target) update && \
		$(opkg_target) install \
			$(call opkg_package_files,$(mkfs_packages_add)))
	-$(CP) -T $(mkfs_cur_target_dir).opkg/ $(mkfs_cur_target_dir)/etc/opkg/
	rm -rf $(mkfs_cur_target_dir).opkg $(mkfs_cur_target_dir).conf
	$(call prepare_rootfs,$(mkfs_cur_target_dir),$(TOPDIR)/files)

$(KDIR)/root.%: kernel_prepare
	$(call Image/mkfs/$(word 1,$(target_params)),$(target_params))

define Device/InitProfile
  PROFILES := $(PROFILE)
  DEVICE_TITLE = $$(DEVICE_VENDOR) $$(DEVICE_MODEL)$$(if $$(DEVICE_VARIANT), $$(DEVICE_VARIANT))
  DEVICE_ALT0_TITLE = $$(DEVICE_ALT0_VENDOR) $$(DEVICE_ALT0_MODEL)$$(if $$(DEVICE_ALT0_VARIANT), $$(DEVICE_ALT0_VARIANT))
  DEVICE_ALT1_TITLE = $$(DEVICE_ALT1_VENDOR) $$(DEVICE_ALT1_MODEL)$$(if $$(DEVICE_ALT1_VARIANT), $$(DEVICE_ALT1_VARIANT))
  DEVICE_ALT2_TITLE = $$(DEVICE_ALT2_VENDOR) $$(DEVICE_ALT2_MODEL)$$(if $$(DEVICE_ALT2_VARIANT), $$(DEVICE_ALT2_VARIANT))
  DEVICE_VENDOR :=
  DEVICE_MODEL :=
  DEVICE_VARIANT :=
  DEVICE_ALT0_VENDOR :=
  DEVICE_ALT0_MODEL :=
  DEVICE_ALT0_VARIANT :=
  DEVICE_ALT1_VENDOR :=
  DEVICE_ALT1_MODEL :=
  DEVICE_ALT1_VARIANT :=
  DEVICE_ALT2_VENDOR :=
  DEVICE_ALT2_MODEL :=
  DEVICE_ALT2_VARIANT :=
  DEVICE_PACKAGES :=
  DEVICE_DESCRIPTION = Build firmware images for $$(DEVICE_TITLE)
endef

define Device/Init
  DEVICE_NAME := $(1)
  KERNEL:=
  KERNEL_INITRAMFS = $$(KERNEL)
  CMDLINE:=

  IMAGES :=
  ARTIFACTS :=
  DEVICE_IMG_PREFIX := $(IMG_PREFIX)-$(1)
  DEVICE_IMG_NAME = $$(DEVICE_IMG_PREFIX)-$$(1)-$$(2)
  IMAGE_SIZE :=
  KERNEL_PREFIX = $$(DEVICE_IMG_PREFIX)
  KERNEL_SUFFIX := -kernel.bin
  KERNEL_INITRAMFS_SUFFIX = $$(KERNEL_SUFFIX)
  KERNEL_IMAGE = $$(KERNEL_PREFIX)$$(KERNEL_SUFFIX)
  KERNEL_INITRAMFS_PREFIX = $$(DEVICE_IMG_PREFIX)-initramfs
  KERNEL_INITRAMFS_IMAGE = $$(KERNEL_INITRAMFS_PREFIX)$$(KERNEL_INITRAMFS_SUFFIX)
  KERNEL_INITRAMFS_NAME = $$(KERNEL_NAME)-initramfs
  KERNEL_INSTALL :=
  KERNEL_NAME := vmlinux
  KERNEL_DEPENDS :=
  KERNEL_SIZE :=

  UBOOTENV_IN_UBI :=
  KERNEL_IN_UBI :=
  BLOCKSIZE :=
  PAGESIZE :=
  SUBPAGESIZE :=
  VID_HDR_OFFSET :=
  UBINIZE_OPTS :=
  UBINIZE_PARTS :=
  MKUBIFS_OPTS :=

  FS_OPTIONS/ubifs = $$(MKUBIFS_OPTS)

  DEVICE_DTS :=
  DEVICE_DTS_CONFIG :=
  DEVICE_DTS_DIR :=
  DEVICE_DTS_OVERLAY :=
  DEVICE_FDT_NUM :=
  SOC :=

  BOARD_NAME :=
  UIMAGE_MAGIC :=
  UIMAGE_NAME :=
  DEVICE_COMPAT_VERSION := 1.0
  DEVICE_COMPAT_MESSAGE :=
  SUPPORTED_DEVICES := $(subst _,$(comma),$(1))
  IMAGE_METADATA :=

  FILESYSTEMS := $(TARGET_FILESYSTEMS)

  UBOOT_PATH :=  $(STAGING_DIR_IMAGE)/uboot-$(1)

  BROKEN :=
  DEFAULT :=
endef

DEFAULT_DEVICE_VARS := \
  DEVICE_NAME KERNEL KERNEL_INITRAMFS KERNEL_INITRAMFS_IMAGE KERNEL_SIZE \
  CMDLINE UBOOTENV_IN_UBI KERNEL_IN_UBI BLOCKSIZE PAGESIZE SUBPAGESIZE \
  VID_HDR_OFFSET UBINIZE_OPTS UBINIZE_PARTS MKUBIFS_OPTS DEVICE_DTS \
  DEVICE_DTS_CONFIG DEVICE_DTS_DIR DEVICE_DTS_OVERLAY DEVICE_FDT_NUM \
  DEVICE_IMG_PREFIX SOC BOARD_NAME UIMAGE_MAGIC UIMAGE_NAME \
  SUPPORTED_DEVICES IMAGE_METADATA KERNEL_ENTRY KERNEL_LOADADDR \
  UBOOT_PATH IMAGE_SIZE \
  DEVICE_PACKAGES DEVICE_COMPAT_VERSION DEVICE_COMPAT_MESSAGE \
  DEVICE_VENDOR DEVICE_MODEL DEVICE_VARIANT \
  DEVICE_ALT0_VENDOR DEVICE_ALT0_MODEL DEVICE_ALT0_VARIANT \
  DEVICE_ALT1_VENDOR DEVICE_ALT1_MODEL DEVICE_ALT1_VARIANT \
  DEVICE_ALT2_VENDOR DEVICE_ALT2_MODEL DEVICE_ALT2_VARIANT

define Device/ExportVar
  $(1) : $(2):=$$($(2))

endef
define Device/Export
  $(foreach var,$(DEVICE_VARS) $(DEFAULT_DEVICE_VARS),$(call Device/ExportVar,$(1),$(var)))
  $(1) : FILESYSTEM:=$(2)
endef

ifdef IB
  DEVICE_CHECK_PROFILE = $(filter $(1),DEVICE_$(PROFILE) $(PROFILE))
else
  DEVICE_CHECK_PROFILE = $(CONFIG_TARGET_$(if $(CONFIG_TARGET_MULTI_PROFILE),DEVICE_)$(call target_conf,$(BOARD)$(if $(SUBTARGET),_$(SUBTARGET)))_$(1))
endif

DEVICE_EXTRA_PACKAGES = $(call qstrip,$(CONFIG_TARGET_DEVICE_PACKAGES_$(call target_conf,$(BOARD)$(if $(SUBTARGET),_$(SUBTARGET)))_DEVICE_$(1)))

define merge_packages
  $(1) :=
  $(foreach pkg,$(2),
    $(1) := $$(strip $$(filter-out -$$(patsubst -%,%,$(pkg)) $$(patsubst -%,%,$(pkg)),$$($(1))) $(pkg))
  )
endef

define Device/Check/Common
  _PROFILE_SET = $$(strip $$(foreach profile,$$(PROFILES) DEVICE_$(1),$$(call DEVICE_CHECK_PROFILE,$$(profile))))
  DEVICE_PACKAGES += $$(call extra_packages,$$(DEVICE_PACKAGES))
  ifdef TARGET_PER_DEVICE_ROOTFS
    $$(eval $$(call merge_packages,_PACKAGES,$$(DEVICE_PACKAGES) $$(call DEVICE_EXTRA_PACKAGES,$(1))))
    ROOTFS_ID/$(1) := $$(if $$(_PROFILE_SET),$$(call mkfs_packages_id,$$(_PACKAGES)))
    PACKAGES_$$(ROOTFS_ID/$(1)) := $$(_PACKAGES)
  endif
endef

define Device/Check
  $(Device/Check/Common)
  KDIR_KERNEL_IMAGE := $(KDIR)/$(1)$$(KERNEL_SUFFIX)
  _TARGET := $$(if $$(_PROFILE_SET),install-images,install-disabled)
  ifndef IB
    _COMPILE_TARGET := $$(if $(CONFIG_IB)$$(_PROFILE_SET),compile,compile-disabled)
  endif
endef

ifndef IB
define Device/Build/initramfs
  $(call Device/Export,$(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE),$(1))
  $$(_TARGET): $$(if $$(KERNEL_INITRAMFS),$(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE) \
	  $$(if $$(CONFIG_JSON_OVERVIEW_IMAGE_INFO), $(BUILD_DIR)/json_info_files/$$(KERNEL_INITRAMFS_IMAGE).json,))

  $(KDIR)/$$(KERNEL_INITRAMFS_NAME):: image_prepare
  $(1)-images: $$(if $$(KERNEL_INITRAMFS),$(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE))
  $(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE): $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE)
	cp $$^ $$@

  $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE): $(KDIR)/$$(KERNEL_INITRAMFS_NAME) $(CURDIR)/Makefile $$(KERNEL_DEPENDS) image_prepare
	@rm -f $$@
	$$(call concat_cmd,$$(KERNEL_INITRAMFS))

  $(call Device/Export,$(BUILD_DIR)/json_info_files/$$(KERNEL_INITRAMFS_IMAGE).json,$(1))

  $(BUILD_DIR)/json_info_files/$$(KERNEL_INITRAMFS_IMAGE).json: $(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE)
	@mkdir -p $$(shell dirname $$@)
	DEVICE_ID="$(1)" \
	BIN_DIR="$(BIN_DIR)" \
	SOURCE_DATE_EPOCH=$(SOURCE_DATE_EPOCH) \
	DEVICE_IMG_NAME="$$(notdir $$^)" \
	IMAGE_TYPE="kernel" \
	IMAGE_FILESYSTEM="initramfs" \
	DEVICE_IMG_PREFIX="$$(DEVICE_IMG_PREFIX)" \
	DEVICE_VENDOR="$$(DEVICE_VENDOR)" \
	DEVICE_MODEL="$$(DEVICE_MODEL)" \
	DEVICE_VARIANT="$$(DEVICE_VARIANT)" \
	DEVICE_ALT0_VENDOR="$$(DEVICE_ALT0_VENDOR)" \
	DEVICE_ALT0_MODEL="$$(DEVICE_ALT0_MODEL)" \
	DEVICE_ALT0_VARIANT="$$(DEVICE_ALT0_VARIANT)" \
	DEVICE_ALT1_VENDOR="$$(DEVICE_ALT1_VENDOR)" \
	DEVICE_ALT1_MODEL="$$(DEVICE_ALT1_MODEL)" \
	DEVICE_ALT1_VARIANT="$$(DEVICE_ALT1_VARIANT)" \
	DEVICE_ALT2_VENDOR="$$(DEVICE_ALT2_VENDOR)" \
	DEVICE_ALT2_MODEL="$$(DEVICE_ALT2_MODEL)" \
	DEVICE_ALT2_VARIANT="$$(DEVICE_ALT2_VARIANT)" \
	DEVICE_TITLE="$$(DEVICE_TITLE)" \
	DEVICE_PACKAGES="$$(DEVICE_PACKAGES)" \
	TARGET="$(BOARD)" \
	SUBTARGET="$(if $(SUBTARGET),$(SUBTARGET),generic)" \
	VERSION_NUMBER="$(VERSION_NUMBER)" \
	VERSION_CODE="$(VERSION_CODE)" \
	SUPPORTED_DEVICES="$$(SUPPORTED_DEVICES)" \
	$(TOPDIR)/scripts/json_add_image_info.py $$@
endef
endif

define Device/Build/compile
  $$(_COMPILE_TARGET): $(KDIR)/$(1)
  $(eval $(call Device/Export,$(KDIR)/$(1)))
  $(KDIR)/$(1):
	$$(call concat_cmd,$(COMPILE/$(1)))

endef

ifndef IB
define Device/Build/dtb
  ifndef BUILD_DTS_$(1)
  BUILD_DTS_$(1) := 1
  $(KDIR)/image-$(1).dtb: FORCE
	$(call Image/BuildDTB,$(strip $(2))/$(strip $(3)).dts,$$@)

  image_prepare: $(KDIR)/image-$(1).dtb
  endif

endef
endif

define Device/Build/kernel
  $$(eval $$(foreach dts,$$(DEVICE_DTS) $$(DEVICE_DTS_OVERLAY), \
	$$(call Device/Build/dtb,$$(notdir $$(dts)), \
		$$(if $$(DEVICE_DTS_DIR),$$(DEVICE_DTS_DIR),$$(DTS_DIR)), \
		$$(dts) \
	) \
  ))

  $(KDIR)/$$(KERNEL_NAME):: image_prepare
  $$(_TARGET): $$(if $$(KERNEL_INSTALL),$(BIN_DIR)/$$(KERNEL_IMAGE))
  $(call Device/Export,$$(KDIR_KERNEL_IMAGE),$(1))
  $(BIN_DIR)/$$(KERNEL_IMAGE): $$(KDIR_KERNEL_IMAGE)
	cp $$^ $$@
  ifndef IB
    ifdef CONFIG_IB
      install: $$(KDIR_KERNEL_IMAGE)
    endif
    $$(KDIR_KERNEL_IMAGE): $(KDIR)/$$(KERNEL_NAME) $(CURDIR)/Makefile $$(KERNEL_DEPENDS) image_prepare
	@rm -f $$@
	$$(call concat_cmd,$$(KERNEL))
	$$(if $$(KERNEL_SIZE),$$(call Build/check-size,$$(KERNEL_SIZE)))
  endif
endef

define Device/Build/image
  GZ_SUFFIX := $(if $(filter %dtb %gz,$(2)),,$(if $(and $(findstring ext4,$(1)),$(CONFIG_TARGET_IMAGES_GZIP)),.gz))
  $$(_TARGET): $(if $(CONFIG_JSON_OVERVIEW_IMAGE_INFO), \
	  $(BUILD_DIR)/json_info_files/$(call DEVICE_IMG_NAME,$(1),$(2)).json, \
	  $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2))$$(GZ_SUFFIX))
  $(eval $(call Device/Export,$(KDIR)/tmp/$(call DEVICE_IMG_NAME,$(1),$(2)),$(1)))
  $(3)-images: $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2))$$(GZ_SUFFIX)

  ROOTFS/$(1)/$(3) := \
	$(KDIR)/root.$(1)$$(strip \
		$$(if $$(FS_OPTIONS/$(1)),+fs=$$(call param_mangle,$$(FS_OPTIONS/$(1)))) \
	)$$(strip \
		$(if $(TARGET_PER_DEVICE_ROOTFS),+pkg=$$(ROOTFS_ID/$(3))) \
	)
  ifndef IB
    $$(ROOTFS/$(1)/$(3)): $(if $(TARGET_PER_DEVICE_ROOTFS),target-dir-$$(ROOTFS_ID/$(3)))
  endif
  $(KDIR)/tmp/$(call DEVICE_IMG_NAME,$(1),$(2)): $$(KDIR_KERNEL_IMAGE) $$(ROOTFS/$(1)/$(3))
	@rm -f $$@
	[ -f $$(word 1,$$^) -a -f $$(word 2,$$^) ]
	$$(call concat_cmd,$(if $(IMAGE/$(2)/$(1)),$(IMAGE/$(2)/$(1)),$(IMAGE/$(2))))

  .IGNORE: $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2))

  $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2)).gz: $(KDIR)/tmp/$(call DEVICE_IMG_NAME,$(1),$(2))
	gzip -c -9n $$^ > $$@

  $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2)): $(KDIR)/tmp/$(call DEVICE_IMG_NAME,$(1),$(2))
	cp $$^ $$@

  $(BUILD_DIR)/json_info_files/$(call DEVICE_IMG_NAME,$(1),$(2)).json: $(BIN_DIR)/$(call DEVICE_IMG_NAME,$(1),$(2))$$(GZ_SUFFIX)
	@mkdir -p $$(shell dirname $$@)
	DEVICE_ID="$(DEVICE_NAME)" \
	BIN_DIR="$(BIN_DIR)" \
	SOURCE_DATE_EPOCH=$(SOURCE_DATE_EPOCH) \
	DEVICE_IMG_NAME="$(DEVICE_IMG_NAME)" \
	IMAGE_TYPE=$(word 1,$(subst ., ,$(2))) \
	IMAGE_FILESYSTEM="$(1)" \
	DEVICE_IMG_PREFIX="$(DEVICE_IMG_PREFIX)" \
	DEVICE_VENDOR="$(DEVICE_VENDOR)" \
	DEVICE_MODEL="$(DEVICE_MODEL)" \
	DEVICE_VARIANT="$(DEVICE_VARIANT)" \
	DEVICE_ALT0_VENDOR="$(DEVICE_ALT0_VENDOR)" \
	DEVICE_ALT0_MODEL="$(DEVICE_ALT0_MODEL)" \
	DEVICE_ALT0_VARIANT="$(DEVICE_ALT0_VARIANT)" \
	DEVICE_ALT1_VENDOR="$(DEVICE_ALT1_VENDOR)" \
	DEVICE_ALT1_MODEL="$(DEVICE_ALT1_MODEL)" \
	DEVICE_ALT1_VARIANT="$(DEVICE_ALT1_VARIANT)" \
	DEVICE_ALT2_VENDOR="$(DEVICE_ALT2_VENDOR)" \
	DEVICE_ALT2_MODEL="$(DEVICE_ALT2_MODEL)" \
	DEVICE_ALT2_VARIANT="$(DEVICE_ALT2_VARIANT)" \
	DEVICE_TITLE="$(DEVICE_TITLE)" \
	DEVICE_PACKAGES="$(DEVICE_PACKAGES)" \
	TARGET="$(BOARD)" \
	SUBTARGET="$(if $(SUBTARGET),$(SUBTARGET),generic)" \
	VERSION_NUMBER="$(VERSION_NUMBER)" \
	VERSION_CODE="$(VERSION_CODE)" \
	SUPPORTED_DEVICES="$(SUPPORTED_DEVICES)" \
	$(TOPDIR)/scripts/json_add_image_info.py $$@

endef

define Device/Build/artifact
  $$(_TARGET): $(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1)
  $(eval $(call Device/Export,$(KDIR)/tmp/$(DEVICE_IMG_PREFIX)-$(1)))
  $(KDIR)/tmp/$(DEVICE_IMG_PREFIX)-$(1): $$(KDIR_KERNEL_IMAGE) $(2)-images
	@rm -f $$@
	$$(call concat_cmd,$(ARTIFACT/$(1)))

  .IGNORE: $(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1)

  $(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1): $(KDIR)/tmp/$(DEVICE_IMG_PREFIX)-$(1)
	cp $$^ $$@

endef

define Device/Build
  $(if $(CONFIG_TARGET_ROOTFS_INITRAMFS),$(call Device/Build/initramfs,$(1)))
  $(call Device/Build/kernel,$(1))

  $$(eval $$(foreach compile,$$(COMPILE), \
    $$(call Device/Build/compile,$$(compile),$(1))))

  $$(eval $$(foreach image,$$(IMAGES), \
    $$(foreach fs,$$(filter $(TARGET_FILESYSTEMS),$$(FILESYSTEMS)), \
      $$(call Device/Build/image,$$(fs),$$(image),$(1)))))

  $$(eval $$(foreach artifact,$$(ARTIFACTS), \
    $$(call Device/Build/artifact,$$(artifact),$(1))))

endef

define Device/DumpInfo
Target-Profile: DEVICE_$(1)
Target-Profile-Name: $(DEVICE_DISPLAY)
Target-Profile-Packages: $(DEVICE_PACKAGES)
Target-Profile-hasImageMetadata: $(if $(foreach image,$(IMAGES),$(findstring append-metadata,$(IMAGE/$(image)))),1,0)
Target-Profile-SupportedDevices: $(SUPPORTED_DEVICES)
$(if $(BROKEN),Target-Profile-Broken: $(BROKEN))
$(if $(DEFAULT),Target-Profile-Default: $(DEFAULT))
Target-Profile-Description:
$(DEVICE_DESCRIPTION)
$(if $(strip $(DEVICE_ALT0_TITLE)),Alternative device titles:
- $(DEVICE_ALT0_TITLE))
$(if $(strip $(DEVICE_ALT1_TITLE)),- $(DEVICE_ALT1_TITLE))
$(if $(strip $(DEVICE_ALT2_TITLE)),- $(DEVICE_ALT2_TITLE))
@@

endef

define Device/Dump
ifneq ($$(strip $$(DEVICE_ALT0_TITLE)),)
DEVICE_DISPLAY = $$(DEVICE_ALT0_TITLE) ($$(DEVICE_TITLE))
$$(info $$(call Device/DumpInfo,$(1)))
endif
ifneq ($$(strip $$(DEVICE_ALT1_TITLE)),)
DEVICE_DISPLAY = $$(DEVICE_ALT1_TITLE) ($$(DEVICE_TITLE))
$$(info $$(call Device/DumpInfo,$(1)))
endif
ifneq ($$(strip $$(DEVICE_ALT2_TITLE)),)
DEVICE_DISPLAY = $$(DEVICE_ALT2_TITLE) ($$(DEVICE_TITLE))
$$(info $$(call Device/DumpInfo,$(1)))
endif
DEVICE_DISPLAY = $$(DEVICE_TITLE)
$$(eval $$(if $$(DEVICE_TITLE),$$(info $$(call Device/DumpInfo,$(1)))))
endef

define Device
  $(call Device/InitProfile,$(1))
  $(call Device/Init,$(1))
  $(call Device/Default,$(1))
  $(call Device/$(1),$(1))
  $(call Device/Check,$(1))
  $(call Device/$(if $(DUMP),Dump,Build),$(1))

endef

define BuildImage

  ifneq ($(DUMP),)
    all: dumpinfo
    dumpinfo: FORCE
	@true
  endif

  download:
  prepare:
  compile:
  clean:
  image_prepare:

  ifeq ($(IB),)
    .PHONY: download prepare compile clean image_prepare kernel_prepare install install-images
    compile:
		$(call Build/Compile)

    clean:
		$(call Build/Clean)

    image_prepare: compile
		mkdir -p $(BIN_DIR) $(KDIR)/tmp
		rm -rf $(BUILD_DIR)/json_info_files
		$(call Image/Prepare)

  else
    image_prepare:
		mkdir -p $(BIN_DIR) $(KDIR)/tmp
  endif

  kernel_prepare: image_prepare
	$(call Image/Build/targz)
	$(call Image/Build/cpiogz)
	$(call Image/BuildKernel)
	$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS),$(if $(IB),,$(call Image/BuildKernel/Initramfs)))
	$(call Image/InstallKernel)

  $(foreach device,$(TARGET_DEVICES),$(call Device,$(device)))

  install-images: kernel_prepare $(foreach fs,$(filter-out $(if $(UBIFS_OPTS),,ubifs),$(TARGET_FILESYSTEMS) $(fs-subtypes-y)),$(KDIR)/root.$(fs))
	$(foreach fs,$(TARGET_FILESYSTEMS),
		$(call Image/Build,$(fs))
	)

  install: install-images
	$(call Image/Manifest)

endef
