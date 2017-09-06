ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
    define Image/mkfs/ubifs/generate
	$(CP) ./ubinize$(1).cfg $(KDIR)
	( cd $(KDIR); \
		$(STAGING_DIR_HOST)/bin/ubinize \
		$(if $($(PROFILE)_UBI_OPTS),$($(PROFILE)_UBI_OPTS),$(shell echo $(UBI_OPTS))) \
		-o $(KDIR)/root$(1).ubi \
		ubinize$(1).cfg \
	)
    endef

    define Image/mkfs/ubifs/legacy

        $(if $($(PROFILE)_UBIFS_OPTS)$(UBIFS_OPTS),
		$(STAGING_DIR_HOST)/bin/mkfs.ubifs \
			$(if $($(PROFILE)_UBIFS_OPTS),$($(PROFILE)_UBIFS_OPTS),$(UBIFS_OPTS)) \
			$(if $(CONFIG_TARGET_UBIFS_FREE_SPACE_FIXUP),--space-fixup) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_NONE),--compr=none) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_LZO),--compr=lzo) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_ZLIB),--compr=zlib) \
			$(if $(shell echo $(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)),--jrn-size=$(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)) \
			--squash-uids \
			-o $(KDIR)/root.ubifs \
			-d $(TARGET_DIR)
	)
	$(call Image/Build,ubifs)

        $(if $($(PROFILE)_UBI_OPTS)$(UBI_OPTS),
		$(if $(wildcard ./ubinize.cfg),$(call Image/mkfs/ubifs/generate,))
		$(if $(wildcard ./ubinize-overlay.cfg),$(call Image/mkfs/ubifs/generate,-overlay))
	)
	$(if $(wildcard ./ubinize.cfg),$(call Image/Build,ubi))
    endef
endif

LegacyDevice/Dump = $(Device/Dump)

define LegacyDevice/Check
  $(Device/Check/Common)
  _TARGET_PREPARE := $$(if $$(_PROFILE_SET),legacy-images-prepare,prepare-disabled)
  _TARGET := $$(if $$(_PROFILE_SET),legacy-images,install-disabled)
  $$(if $$(_PROFILE_SET),install: legacy-images-make)
  ifndef IB
    $$(if $$(_PROFILE_SET),kernel_prepare: legacy-images-prepare-make)
  endif
endef

ifdef TARGET_PER_DEVICE_ROOTFS
  define Image/Build/Profile/Filesystem
	cp $(KDIR)/root.$(2)+pkg=$(3) $(KDIR)/root.$(2)
	$(call Image/Build/$(2),$(2))
	$(call Image/Build/Profile,$(1),$(2))
  endef
else
  Image/Build/Profile/Filesystem = $(Image/Build/Profile)
endif

define LegacyDevice/Build
  $$(_TARGET): legacy-image-$(1)
  $$(_TARGET_PREPARE): legacy-image-prepare-$(1)
  .PHONY: legacy-image-prepare-$(1) legacy-image-$(1)

  legacy-image-prepare-$(1):
	$$(call Image/Prepare/Profile,$(1))

  ifndef IB
    ifdef CONFIG_TARGET_PER_DEVICE_ROOTFS
      ROOTFS/$(1) := $(foreach fs,$(TARGET_FILESYSTEMS), \
        $(KDIR)/root.$(fs)$$(strip $(if $(CONFIG_TARGET_PER_DEVICE_ROOTFS),+pkg=$$(ROOTFS_ID/$(1)))) \
      )

      $$(ROOTFS/$(1)): target-dir-$$(ROOTFS_ID/$(1))
      legacy-images-make: $$(if $$(_PROFILE_SET),$$(ROOTFS/$(1)))
    endif
  endif

  legacy-image-$(1):
	$$(call Image/BuildKernel/Profile,$(1))
	$(foreach fs,$(TARGET_FILESYSTEMS),
		$$(call Image/Build/Profile/Filesystem,$(1),$(fs),$$(ROOTFS_ID/$(1)))
	)

endef

define LegacyDevice
  $(call Device/InitProfile,$(1))
  $(call Device/Default,$(1))
  $(call LegacyDevice/Default,$(1))
  $(call LegacyDevice/$(1),$(1))
  $(call LegacyDevice/Check,$(1))
  $(call LegacyDevice/$(if $(DUMP),Dump,Build),$(1))

endef
