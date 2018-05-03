# Build commands that can be called from Device/* templates

IMAGE_KERNEL = $(word 1,$^)
IMAGE_ROOTFS = $(word 2,$^)

define Build/uImage
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(if $(UIMAGE_NAME),$(UIMAGE_NAME),$(call toupper,$(LINUX_KARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION))' -d $@ $@.new
	mv $@.new $@
endef

define Build/buffalo-enc
	$(eval product=$(word 1,$(1)))
	$(eval version=$(word 2,$(1)))
	$(eval args=$(wordlist 3,$(words $(1)),$(1)))
	$(STAGING_DIR_HOST)/bin/buffalo-enc \
		-p $(product) -v $(version) $(args) \
		-i $@ -o $@.new
	mv $@.new $@
endef

define Build/buffalo-enc-tag
	$(call Build/buffalo-enc,'' '' -S 152 $(1))
endef

define Build/buffalo-tag-dhp
	$(eval product=$(word 1,$(1)))
	$(eval region=$(word 2,$(1)))
	$(eval language=$(word 3,$(1)))
	$(STAGING_DIR_HOST)/bin/buffalo-tag \
		-d 0x01000000 -w 1 \
		-a $(BUFFALO_TAG_PLATFORM) \
		-v $(BUFFALO_TAG_VERSION) -m $(BUFFALO_TAG_MINOR) \
		-b $(product) -p $(product) \
		-r $(region) -r $(region) -l $(language) \
		-I $@ -o $@.new
	mv $@.new $@
endef

define Build/buffalo-dhp-image
	$(STAGING_DIR_HOST)/bin/mkdhpimg $@ $@.new
	mv $@.new $@
endef

define Build/eva-image
	$(STAGING_DIR_HOST)/bin/lzma2eva $(KERNEL_LOADADDR) $(KERNEL_LOADADDR) $@ $@.new
	mv $@.new $@
endef

define Build/netgear-chk
	$(STAGING_DIR_HOST)/bin/mkchkimg \
		-o $@.new \
		-k $@ \
		-b $(NETGEAR_BOARD_ID) \
		$(if $(NETGEAR_REGION),-r $(NETGEAR_REGION),)
	mv $@.new $@
endef

define Build/netgear-dni
	$(STAGING_DIR_HOST)/bin/mkdniimg \
		-B $(NETGEAR_BOARD_ID) -v $(VERSION_DIST).$(REVISION) \
		$(if $(NETGEAR_HW_ID),-H $(NETGEAR_HW_ID)) \
		-r "$(1)" \
		-i $@ -o $@.new
	mv $@.new $@
endef

define Build/append-squashfs-fakeroot-be
	rm -rf $@.fakefs $@.fakesquashfs
	mkdir $@.fakefs
	$(STAGING_DIR_HOST)/bin/mksquashfs-lzma \
		$@.fakefs $@.fakesquashfs \
		-noappend -root-owned -be -nopad -b 65536 \
		$(if $(SOURCE_DATE_EPOCH),-fixed-time $(SOURCE_DATE_EPOCH))
	cat $@.fakesquashfs >> $@
endef

# append a fake/empty uImage header, to fool bootloaders rootfs integrity check
# for example
define Build/append-uImage-fakehdr
	touch $@.fakehdr
	$(STAGING_DIR_HOST)/bin/mkimage \
		-A $(LINUX_KARCH) -O linux -T $(1) -C none \
		-n '$(VERSION_DIST) fake $(1)' \
		-d $@.fakehdr \
		-s \
		$@.fakehdr
	cat $@.fakehdr >> $@
endef

define Build/tplink-safeloader
       -$(STAGING_DIR_HOST)/bin/tplink-safeloader \
		-B $(TPLINK_BOARD_ID) \
		-V $(REVISION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@.new \
		-j \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-S) && mv $@.new $@ || rm -f $@
endef

define Build/append-dtb
	cat $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb >> $@
endef

define Build/install-dtb
	$(foreach dts,$(DEVICE_DTS), \
		$(CP) \
			$(DTS_DIR)/$(dts).dtb \
			$(BIN_DIR)/$(IMG_PREFIX)-$(dts).dtb; \
	)
endef

define Build/fit
	$(TOPDIR)/scripts/mkits.sh \
		-D $(DEVICE_NAME) -o $@.its -k $@ \
		$(if $(word 2,$(1)),-d $(word 2,$(1))) -C $(word 1,$(1)) \
		-a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-c $(if $(DEVICE_DTS_CONFIG),$(DEVICE_DTS_CONFIG),"config@1") \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/lzma
	$(call Build/lzma-no-dict,-lc1 -lp2 -pb2 $(1))
endef

define Build/lzma-no-dict
	$(STAGING_DIR_HOST)/bin/lzma e $@ $(1) $@.new
	@mv $@.new $@
endef

define Build/gzip
	gzip --force -9n -c $@ $(1) > $@.new
	@mv $@.new $@
endef

define Build/jffs2
	rm -rf $(KDIR_TMP)/$(DEVICE_NAME)/jffs2 && \
		mkdir -p $(KDIR_TMP)/$(DEVICE_NAME)/jffs2/$$(dirname $(1)) && \
		cp $@ $(KDIR_TMP)/$(DEVICE_NAME)/jffs2/$(1) && \
		$(STAGING_DIR_HOST)/bin/mkfs.jffs2 --pad \
			$(if $(CONFIG_BIG_ENDIAN),--big-endian,--little-endian) \
			--squash-uids -v -e $(patsubst %k,%KiB,$(BLOCKSIZE)) \
			-o $@.new \
			-d $(KDIR_TMP)/$(DEVICE_NAME)/jffs2 \
			2>&1 1>/dev/null | awk '/^.+$$$$/' && \
		$(STAGING_DIR_HOST)/bin/padjffs2 $@.new -J $(patsubst %k,,$(BLOCKSIZE))
	-rm -rf $(KDIR_TMP)/$(DEVICE_NAME)/jffs2/
	@mv $@.new $@
endef

define Build/kernel-bin
	rm -f $@
	cp $< $@
endef

define Build/patch-cmdline
	$(STAGING_DIR_HOST)/bin/patch-cmdline $@ '$(CMDLINE)'
endef

define Build/append-kernel
	dd if=$(IMAGE_KERNEL) >> $@
endef

define Build/append-rootfs
	dd if=$(IMAGE_ROOTFS) >> $@
endef

define Build/append-ubi
	sh $(TOPDIR)/scripts/ubinize-image.sh \
		$(if $(UBOOTENV_IN_UBI),--uboot-env) \
		$(if $(KERNEL_IN_UBI),--kernel $(IMAGE_KERNEL)) \
		$(foreach part,$(UBINIZE_PARTS),--part $(part)) \
		$(IMAGE_ROOTFS) \
		$@.tmp \
		-p $(BLOCKSIZE:%k=%KiB) -m $(PAGESIZE) \
		$(if $(SUBPAGESIZE),-s $(SUBPAGESIZE)) \
		$(if $(VID_HDR_OFFSET),-O $(VID_HDR_OFFSET)) \
		$(UBINIZE_OPTS)
	cat $@.tmp >> $@
	rm $@.tmp
endef

define Build/pad-to
	dd if=$@ of=$@.new bs=$(1) conv=sync
	mv $@.new $@
endef

define Build/pad-extra
	dd if=/dev/zero bs=$(1) count=1 >> $@
endef

define Build/pad-rootfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $@ $(1) \
		$(if $(BLOCKSIZE),$(BLOCKSIZE:%k=%),4 8 16 64 128 256)
endef

define Build/pad-offset
	let \
		size="$$(stat -c%s $@)" \
		pad="$(subst k,* 1024,$(word 1, $(1)))" \
		offset="$(subst k,* 1024,$(word 2, $(1)))" \
		pad="(pad - ((size + offset) % pad)) % pad" \
		newsize='size + pad'; \
		dd if=$@ of=$@.new bs=$$newsize count=1 conv=sync
	mv $@.new $@
endef

define Build/check-size
	@[ $$(($(subst k,* 1024,$(subst m, * 1024k,$(1))))) -ge "$$(stat -c%s $@)" ] || { \
		echo "WARNING: Image file $@ is too big" >&2; \
		rm -f $@; \
	}
endef

define Build/combined-image
	-sh $(TOPDIR)/scripts/combined-image.sh \
		"$(IMAGE_KERNEL)" \
		"$@" \
		"$@.new"
	@mv $@.new $@
endef

define Build/openmesh-image
	$(TOPDIR)/scripts/om-fwupgradecfg-gen.sh \
		"$(call param_get_default,ce_type,$(1),$(DEVICE_NAME))" \
		"$@-fwupgrade.cfg" \
		"$(call param_get_default,kernel,$(1),$(IMAGE_KERNEL))" \
		"$(call param_get_default,rootfs,$(1),$@)"
	$(TOPDIR)/scripts/combined-ext-image.sh \
		"$(call param_get_default,ce_type,$(1),$(DEVICE_NAME))" "$@" \
		"$@-fwupgrade.cfg" "fwupgrade.cfg" \
		"$(call param_get_default,kernel,$(1),$(IMAGE_KERNEL))" "kernel" \
		"$(call param_get_default,rootfs,$(1),$@)" "rootfs"
endef

define Build/sysupgrade-tar
	sh $(TOPDIR)/scripts/sysupgrade-tar.sh \
		--board $(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)) \
		--kernel $(call param_get_default,kernel,$(1),$(IMAGE_KERNEL)) \
		--rootfs $(call param_get_default,rootfs,$(1),$(IMAGE_ROOTFS)) \
		$@
endef

define Build/tplink-v1-header
	$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-c -H $(TPLINK_HWID) -W $(TPLINK_HWREV) -L $(KERNEL_LOADADDR) \
		-E $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-m $(TPLINK_HEADER_VERSION) -N "$(VERSION_DIST)" -V $(REVISION) \
		-k $@ -o $@.new $(1)
	@mv $@.new $@
endef

define Build/tplink-v2-header
	$(STAGING_DIR_HOST)/bin/mktplinkfw2 \
		-c -H $(TPLINK_HWID) -W $(TPLINK_HWREV) -L $(KERNEL_LOADADDR) \
		-E $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR))  \
		-w $(TPLINK_HWREVADD) -F "$(TPLINK_FLASHLAYOUT)" \
		-T $(TPLINK_HVERSION) -V "ver. 2.0" \
		-k $@ -o $@.new $(1)
	@mv $@.new $@
endef

define Build/tplink-v2-image
	$(STAGING_DIR_HOST)/bin/mktplinkfw2 \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) \
		-w $(TPLINK_HWREVADD) -F "$(TPLINK_FLASHLAYOUT)" \
		-T $(TPLINK_HVERSION) -V "ver. 2.0" -a 0x4 -j \
		-k $(IMAGE_KERNEL) -r $(IMAGE_ROOTFS) -o $@.new $(1)
	cat $@.new >> $@
	rm -rf $@.new
endef

json_quote=$(subst ','\'',$(subst ",\",$(1)))
#")')
metadata_devices=$(if $(1),$(subst "$(space)","$(comma)",$(strip $(foreach v,$(1),"$(call json_quote,$(v))"))))
metadata_json = \
	'{ $(if $(IMAGE_METADATA),$(IMAGE_METADATA)$(comma)) \
		"supported_devices":[$(call metadata_devices,$(1))], \
		"version": { \
			"dist": "$(call json_quote,$(VERSION_DIST))", \
			"version": "$(call json_quote,$(VERSION_NUMBER))", \
			"revision": "$(call json_quote,$(REVISION))", \
			"board": "$(call json_quote,$(BOARD))" \
		} \
	}'

define Build/append-metadata
	$(if $(SUPPORTED_DEVICES),-echo $(call metadata_json,$(SUPPORTED_DEVICES)) | fwtool -I - $@)
endef

define Build/kernel2minor
	kernel2minor -k $@ -r $@.new $(1)
	mv $@.new $@
endef
