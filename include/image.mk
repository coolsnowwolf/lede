#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

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

include $(INCLUDE_DIR)/image-legacy.mk
include $(INCLUDE_DIR)/feeds.mk
include $(INCLUDE_DIR)/rootfs.mk

override MAKE:=$(_SINGLE)$(SUBMAKE)
override NO_TRACE_MAKE:=$(_SINGLE)$(NO_TRACE_MAKE)

target_params = $(subst +,$(space),$*)
param_get = $(patsubst $(1)=%,%,$(filter $(1)=%,$(2)))
param_get_default = $(firstword $(call param_get,$(1),$(2)) $(3))
param_mangle = $(subst $(space),_,$(strip $(1)))
param_unmangle = $(subst _,$(space),$(1))

mkfs_packages_id = $(shell echo $(sort $(1)) | mkhash md5 | head -c 8)
mkfs_target_dir = $(if $(call param_get,pkg,$(1)),$(KDIR)/target-dir-$(call param_get,pkg,$(1)),$(TARGET_DIR))

KDIR=$(KERNEL_BUILD_DIR)
KDIR_TMP=$(KDIR)/tmp
DTS_DIR:=$(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts

IMG_PREFIX_EXTRA:=$(if $(EXTRA_IMAGE_NAME),$(call sanitize,$(EXTRA_IMAGE_NAME))-)
IMG_PREFIX_VERNUM:=$(if $(CONFIG_VERSION_FILENAMES),$(call sanitize,$(VERSION_NUMBER))-)
IMG_PREFIX_VERCODE:=$(if $(CONFIG_VERSION_CODE_FILENAMES),$(call sanitize,$(VERSION_CODE))-)

IMG_PREFIX:=$(VERSION_DIST_SANITIZED)-$(IMG_PREFIX_VERNUM)$(IMG_PREFIX_VERCODE)$(IMG_PREFIX_EXTRA)$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))

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
fs-types-$(CONFIG_TARGET_ROOTFS_ISO) += iso
fs-types-$(CONFIG_TARGET_ROOTFS_UBIFS) += ubifs
fs-subtypes-$(CONFIG_TARGET_ROOTFS_JFFS2) += $(addsuffix -raw,$(addprefix jffs2-,$(JFFS2_BLOCKSIZE)))

TARGET_FILESYSTEMS := $(fs-types-y)

FS_64K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-64k
FS_128K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-128k
FS_256K := $(filter-out jffs2-%,$(TARGET_FILESYSTEMS)) jffs2-256k

define add_jffs2_mark
	echo -ne '\xde\xad\xc0\xde' >> $(1)
endef

PROFILE_SANITIZED := $(call sanitize,$(PROFILE))

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
		-c $(if $(DEVICE_DTS_CONFIG),$(DEVICE_DTS_CONFIG),"config@1") \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1)$(7).itb
endef

ifdef CONFIG_TARGET_IMAGES_GZIP
  define Image/Gzip
	rm -f $(1).gz
	gzip -9n $(1)
  endef
endif

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
		-i$(dir $(1)) $(4) \
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

define Image/mkfs/squashfs
	$(STAGING_DIR_HOST)/bin/mksquashfs4 $(call mkfs_target_dir,$(1)) $@ \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) $(SQUASHFSOPT) \
		-processors 1 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))
endef

# $(1): board name
# $(2): rootfs type
# $(3): kernel image
# $(4): compat string
ifneq ($(CONFIG_NAND_SUPPORT),)
   define Image/Build/SysupgradeNAND
	mkdir -p "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/"
	echo "BOARD=$(if $(4),$(4),$(1))" > "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/CONTROL"
	[ -z "$(2)" ] || $(CP) "$(KDIR)/root.$(2)" "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/root"
	[ -z "$(3)" ] || $(CP) "$(3)" "$(KDIR_TMP)/sysupgrade-$(if $(4),$(4),$(1))/kernel"
	(cd "$(KDIR_TMP)"; $(TAR) cvf \
		"$(BIN_DIR)/$(IMG_PREFIX)-$(1)-$(2)-sysupgrade.tar" sysupgrade-$(if $(4),$(4),$(1)) \
			$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
	)
   endef

# $(1) board name
# $(2) ubinize-image options (e.g. --uboot-env and/or --kernel kernelimage)
# $(3) rootfstype (e.g. squashfs or ubifs)
# $(4) options to pass-through to ubinize (i.e. $($(PROFILE)_UBI_OPTS)))
   define Image/Build/UbinizeImage
	sh $(TOPDIR)/scripts/ubinize-image.sh $(2) \
		"$(KDIR)/root.$(3)" \
		"$(KDIR)/$(IMG_PREFIX)-$(1)-$(3)-ubinized.bin" \
		$(4)
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

E2SIZE=$(shell echo $$(($(CONFIG_TARGET_ROOTFS_PARTSIZE)*1024*1024)))

define Image/mkfs/ext4
	$(STAGING_DIR_HOST)/bin/make_ext4fs \
		-l $(E2SIZE) -b $(CONFIG_TARGET_EXT4_BLOCKSIZE) \
		$(if $(CONFIG_TARGET_EXT4_RESERVED_PCT),-m $(CONFIG_TARGET_EXT4_RESERVED_PCT)) \
		$(if $(CONFIG_TARGET_EXT4_JOURNAL),,-J) \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		$@ $(call mkfs_target_dir,$(1))/
endef

define Image/Manifest
	$(call opkg,$(TARGET_DIR_ORIG)) list-installed > \
		$(BIN_DIR)/$(IMG_PREFIX)$(if $(PROFILE_SANITIZED),-$(PROFILE_SANITIZED)).manifest
endef

ifdef CONFIG_TARGET_ROOTFS_TARGZ
  define Image/Build/targz
	$(TAR) -cp --numeric-owner --owner=0 --group=0 --sort=name \
		$(if $(SOURCE_DATE_EPOCH),--mtime="@$(SOURCE_DATE_EPOCH)") \
		-C $(TARGET_DIR)/ . | gzip -9n > $(BIN_DIR)/$(IMG_PREFIX)$(if $(PROFILE_SANITIZED),-$(PROFILE_SANITIZED))-rootfs.tar.gz
  endef
endif

ifdef CONFIG_TARGET_ROOTFS_CPIOGZ
  define Image/Build/cpiogz
	( cd $(TARGET_DIR); find . | cpio -o -H newc -R root:root | gzip -9n >$(BIN_DIR)/$(IMG_PREFIX)-rootfs.cpio.gz )
  endef
endif

mkfs_packages = $(filter-out @%,$(PACKAGES_$(call param_get,pkg,pkg=$(target_params))))
mkfs_packages_add = $(filter-out -%,$(mkfs_packages))
mkfs_packages_remove = $(patsubst -%,%,$(filter -%,$(mkfs_packages)))
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
  DEVICE_TITLE :=
  DEVICE_PACKAGES :=
  DEVICE_DESCRIPTION = Build firmware images for $$(DEVICE_TITLE)
endef

define Device/Init
  DEVICE_NAME := $(1)
  KERNEL:=
  KERNEL_INITRAMFS = $$(KERNEL)
  KERNEL_SIZE:=
  CMDLINE:=

  IMAGES :=
  IMAGE_PREFIX := $(IMG_PREFIX)-$(1)
  IMAGE_NAME = $$(IMAGE_PREFIX)-$$(1)-$$(2)
  KERNEL_PREFIX = $$(IMAGE_PREFIX)
  KERNEL_SUFFIX := -kernel.bin
  KERNEL_INITRAMFS_SUFFIX = $$(KERNEL_SUFFIX)
  KERNEL_IMAGE = $$(KERNEL_PREFIX)$$(KERNEL_SUFFIX)
  KERNEL_INITRAMFS_PREFIX = $$(IMAGE_PREFIX)-initramfs
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

  BOARD_NAME :=
  UIMAGE_NAME :=
  SUPPORTED_DEVICES :=
  IMAGE_METADATA :=

  FILESYSTEMS := $(TARGET_FILESYSTEMS)
endef

DEFAULT_DEVICE_VARS := \
  DEVICE_NAME KERNEL KERNEL_INITRAMFS KERNEL_SIZE KERNEL_INITRAMFS_IMAGE \
  KERNEL_LOADADDR DEVICE_DTS DEVICE_DTS_CONFIG DEVICE_DTS_DIR BOARD_NAME \
  CMDLINE UBOOTENV_IN_UBI KERNEL_IN_UBI \
  BLOCKSIZE PAGESIZE SUBPAGESIZE VID_HDR_OFFSET \
  UBINIZE_OPTS UIMAGE_NAME UBINIZE_PARTS \
  SUPPORTED_DEVICES IMAGE_METADATA

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
  $$(_TARGET): $$(if $$(KERNEL_INITRAMFS),$(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE))

  $(KDIR)/$$(KERNEL_INITRAMFS_NAME):: image_prepare
  $(BIN_DIR)/$$(KERNEL_INITRAMFS_IMAGE): $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE)
	cp $$^ $$@

  $(KDIR)/tmp/$$(KERNEL_INITRAMFS_IMAGE): $(KDIR)/$$(KERNEL_INITRAMFS_NAME) $(CURDIR)/Makefile $$(KERNEL_DEPENDS) image_prepare
	@rm -f $$@
	$$(call concat_cmd,$$(KERNEL_INITRAMFS))
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
  $$(eval $$(foreach dts,$$(DEVICE_DTS), \
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
  $$(_TARGET): $(BIN_DIR)/$(call IMAGE_NAME,$(1),$(2))$$(GZ_SUFFIX)
  $(eval $(call Device/Export,$(KDIR)/tmp/$(call IMAGE_NAME,$(1),$(2)),$(1)))
  ROOTFS/$(1)/$(3) := \
	$(KDIR)/root.$(1)$$(strip \
		$$(if $$(FS_OPTIONS/$(1)),+fs=$$(call param_mangle,$$(FS_OPTIONS/$(1)))) \
	)$$(strip \
		$(if $(TARGET_PER_DEVICE_ROOTFS),+pkg=$$(ROOTFS_ID/$(3))) \
	)
  ifndef IB
    $$(ROOTFS/$(1)/$(3)): $(if $(TARGET_PER_DEVICE_ROOTFS),target-dir-$$(ROOTFS_ID/$(3)))
  endif
  $(KDIR)/tmp/$(call IMAGE_NAME,$(1),$(2)): $$(KDIR_KERNEL_IMAGE) $$(ROOTFS/$(1)/$(3))
	@rm -f $$@
	[ -f $$(word 1,$$^) -a -f $$(word 2,$$^) ]
	$$(call concat_cmd,$(if $(IMAGE/$(2)/$(1)),$(IMAGE/$(2)/$(1)),$(IMAGE/$(2))))

  .IGNORE: $(BIN_DIR)/$(call IMAGE_NAME,$(1),$(2))

  $(BIN_DIR)/$(call IMAGE_NAME,$(1),$(2)).gz: $(KDIR)/tmp/$(call IMAGE_NAME,$(1),$(2))
	gzip -c -9n $$^ > $$@

  $(BIN_DIR)/$(call IMAGE_NAME,$(1),$(2)): $(KDIR)/tmp/$(call IMAGE_NAME,$(1),$(2))
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
endef

define Device/DumpInfo
Target-Profile: DEVICE_$(1)
Target-Profile-Name: $(DEVICE_TITLE)
Target-Profile-Packages: $(DEVICE_PACKAGES)
Target-Profile-Description:
$(DEVICE_DESCRIPTION)
@@

endef

define Device/Dump
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
  legacy-images-prepare:
  legacy-images:
  image_prepare:

  ifeq ($(IB),)
    .PHONY: download prepare compile clean image_prepare kernel_prepare install install-images
    compile:
		$(call Build/Compile)

    clean:
		$(call Build/Clean)

    image_prepare: compile
		mkdir -p $(BIN_DIR) $(KDIR)/tmp
		$(call Image/Prepare)

    legacy-images-prepare-make: image_prepare
		$(MAKE) legacy-images-prepare

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
  $(foreach device,$(LEGACY_DEVICES),$(call LegacyDevice,$(device)))

  install-images: kernel_prepare $(foreach fs,$(filter-out $(if $(UBIFS_OPTS),,ubifs),$(TARGET_FILESYSTEMS) $(fs-subtypes-y)),$(KDIR)/root.$(fs))
	$(foreach fs,$(TARGET_FILESYSTEMS),
		$(call Image/Build,$(fs))
	)

  legacy-images-make: install-images
	$(call Image/mkfs/ubifs/legacy)
	$(MAKE) legacy-images

  install: install-images
	$(call Image/Manifest)

endef
