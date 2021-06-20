# Build commands that can be called from Device/* templates

IMAGE_KERNEL = $(word 1,$^)
IMAGE_ROOTFS = $(word 2,$^)

define ModelNameLimit16
$(shell expr substr "$(word 2, $(subst _, ,$(1)))" 1 16)
endef

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs%,0x4,$(patsubst root.%,%,$(1))))))
endef


define Build/append-dtb
	cat $(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb >> $@
endef

define Build/append-dtb-elf
	$(TARGET_CROSS)objcopy \
		--set-section-flags=.appended_dtb=alloc,contents \
		--update-section \
		.appended_dtb=$(KDIR)/image-$(firstword $(DEVICE_DTS)).dtb $@
endef

define Build/append-kernel
	dd if=$(IMAGE_KERNEL) >> $@
endef

define Build/append-image
	dd if=$(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1) >> $@
endef

ifdef IB
define Build/append-image-stage
	dd if=$(STAGING_DIR_IMAGE)/$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))-$(DEVICE_NAME)-$(1) >> $@
endef
else
define Build/append-image-stage
	dd if=$(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1) of=$(STAGING_DIR_IMAGE)/$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))-$(DEVICE_NAME)-$(1)
	dd if=$(BIN_DIR)/$(DEVICE_IMG_PREFIX)-$(1) >> $@
endef
endif


compat_version=$(if $(DEVICE_COMPAT_VERSION),$(DEVICE_COMPAT_VERSION),1.0)
json_quote=$(subst ','\'',$(subst ",\",$(1)))
#")')

legacy_supported_message=$(SUPPORTED_DEVICES) - Image version mismatch: image $(compat_version), \
	device 1.0. Please wipe config during upgrade (force required) or reinstall. \
	$(if $(DEVICE_COMPAT_MESSAGE),Reason: $(DEVICE_COMPAT_MESSAGE),Please check documentation ...)

metadata_devices=$(if $(1),$(subst "$(space)","$(comma)",$(strip $(foreach v,$(1),"$(call json_quote,$(v))"))))
metadata_json = \
	'{ $(if $(IMAGE_METADATA),$(IMAGE_METADATA)$(comma)) \
		"metadata_version": "1.1", \
		"compat_version": "$(call json_quote,$(compat_version))", \
		$(if $(DEVICE_COMPAT_MESSAGE),"compat_message": "$(call json_quote,$(DEVICE_COMPAT_MESSAGE))"$(comma)) \
		$(if $(filter-out 1.0,$(compat_version)),"new_supported_devices": \
			[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma) \
			"supported_devices": ["$(call json_quote,$(legacy_supported_message))"]$(comma)) \
		$(if $(filter 1.0,$(compat_version)),"supported_devices":[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma)) \
		"version": { \
			"dist": "$(call json_quote,$(VERSION_DIST))", \
			"version": "$(call json_quote,$(VERSION_NUMBER))", \
			"revision": "$(call json_quote,$(REVISION))", \
			"target": "$(call json_quote,$(TARGETID))", \
			"board": "$(call json_quote,$(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)))" \
		} \
	}'

define Build/append-metadata
	$(if $(SUPPORTED_DEVICES),-echo $(call metadata_json) | fwtool -I - $@)
	[ ! -s "$(BUILD_KEY)" -o ! -s "$(BUILD_KEY).ucert" -o ! -s "$@" ] || { \
		cp "$(BUILD_KEY).ucert" "$@.ucert" ;\
		usign -S -m "$@" -s "$(BUILD_KEY)" -x "$@.sig" ;\
		ucert -A -c "$@.ucert" -x "$@.sig" ;\
		fwtool -S "$@.ucert" "$@" ;\
	}
endef

define Build/append-rootfs
	dd if=$(IMAGE_ROOTFS) >> $@
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

define Build/append-string
	echo -n $(1) >> $@
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

define Build/append-uboot
	dd if=$(UBOOT_PATH) >> $@
endef

# append a fake/empty uImage header, to fool bootloaders rootfs integrity check
# for example
define Build/append-uImage-fakehdr
	$(eval type=$(word 1,$(1)))
	$(eval magic=$(word 2,$(1)))
	touch $@.fakehdr
	$(STAGING_DIR_HOST)/bin/mkimage \
		-A $(LINUX_KARCH) -O linux -T $(type) -C none \
		-n '$(VERSION_DIST) fake $(type)' \
		$(if $(magic),-M $(magic)) \
		-d $@.fakehdr \
		-s \
		$@.fakehdr
	cat $@.fakehdr >> $@
endef

define Build/buffalo-dhp-image
	$(STAGING_DIR_HOST)/bin/mkdhpimg $@ $@.new
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

define Build/check-size
	@imagesize="$$(stat -c%s $@)"; \
	limitsize="$$(($(subst k,* 1024,$(subst m, * 1024k,$(if $(1),$(1),$(IMAGE_SIZE))))))"; \
	[ $$limitsize -ge $$imagesize ] || { \
		echo "WARNING: Image file $@ is too big: $$imagesize > $$limitsize" >&2; \
		rm -f $@; \
	}
endef

define Build/elecom-product-header
	$(eval product=$(word 1,$(1)))
	$(eval fw=$(if $(word 2,$(1)),$(word 2,$(1)),$@))

	( \
		echo -n -e "ELECOM\x00\x00$(product)" | dd bs=40 count=1 conv=sync; \
		echo -n "0.00" | dd bs=16 count=1 conv=sync; \
		dd if=$(fw); \
	) > $(fw).new
	mv $(fw).new $(fw)
endef

define Build/elx-header
	$(eval hw_id=$(word 1,$(1)))
	$(eval xor_pattern=$(word 2,$(1)))
	( \
		echo -ne "\x00\x00\x00\x00\x00\x00\x00\x03" | \
			dd bs=42 count=1 conv=sync; \
		hw_id="$(hw_id)"; \
		echo -ne "\x$${hw_id:0:2}\x$${hw_id:2:2}\x$${hw_id:4:2}\x$${hw_id:6:2}" | \
			dd bs=20 count=1 conv=sync; \
		echo -ne "$$(printf '%08x' $$(stat -c%s $@) | fold -s2 | xargs -I {} echo \\x{} | tr -d '\n')" | \
			dd bs=8 count=1 conv=sync; \
		echo -ne "$$($(MKHASH) md5 $@ | fold -s2 | xargs -I {} echo \\x{} | tr -d '\n')" | \
			dd bs=58 count=1 conv=sync; \
	) > $(KDIR)/tmp/$(DEVICE_NAME).header
	$(call Build/xor-image,-p $(xor_pattern) -x)
	cat $(KDIR)/tmp/$(DEVICE_NAME).header $@ > $@.new
	mv $@.new $@
	rm -rf $(KDIR)/tmp/$(DEVICE_NAME).header
endef

define Build/eva-image
	$(STAGING_DIR_HOST)/bin/lzma2eva $(KERNEL_LOADADDR) $(KERNEL_LOADADDR) $@ $@.new
	mv $@.new $@
endef

define Build/initrd_compression
	$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_BZIP2),.bzip2) \
	$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_GZIP),.gzip) \
	$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_LZMA),.lzma) \
	$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_XZ),.xz) \
	$(if $(CONFIG_TARGET_INITRAMFS_COMPRESSION_ZSTD),.zstd)
endef

define Build/fit
	$(TOPDIR)/scripts/mkits.sh \
		-D $(DEVICE_NAME) -o $@.its -k $@ \
		-C $(word 1,$(1)) $(if $(word 2,$(1)),\
		$(if $(DEVICE_DTS_OVERLAY),-d $(KERNEL_BUILD_DIR)/image-$$(basename $(word 2,$(1))),\
			-d $(word 2,$(1)))) \
		$(if $(findstring with-rootfs,$(word 3,$(1))),-r $(IMAGE_ROOTFS)) \
		$(if $(findstring with-initrd,$(word 3,$(1))), \
			$(if $(CONFIG_TARGET_ROOTFS_INITRAMFS_SEPARATE), \
				-i $(KERNEL_BUILD_DIR)/initrd.cpio$(strip $(call Build/initrd_compression)))) \
		-a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		$(if $(DEVICE_FDT_NUM),-n $(DEVICE_FDT_NUM)) \
		$(if $(DEVICE_DTS_OVERLAY),$(foreach dtso,$(DEVICE_DTS_OVERLAY), -O $(dtso):$(KERNEL_BUILD_DIR)/image-$(dtso).dtb)) \
		-c $(if $(DEVICE_DTS_CONFIG),$(DEVICE_DTS_CONFIG),"config-1") \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage $(if $(findstring external,$(word 3,$(1))),\
		-E -B 0x1000 $(if $(findstring static,$(word 3,$(1))),-p 0x1000)) -f $@.its $@.new
	@mv $@.new $@
endef

define Build/gzip
	gzip -f -9n -c $@ $(1) > $@.new
	@mv $@.new $@
endef

define Build/install-dtb
	$(call locked, \
		$(foreach dts,$(DEVICE_DTS), \
			$(CP) \
				$(DTS_DIR)/$(dts).dtb \
				$(BIN_DIR)/$(IMG_PREFIX)-$(dts).dtb; \
		), \
		install-dtb-$(IMG_PREFIX) \
	)
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

define Build/kernel2minor
	$(eval temp_file := $(shell mktemp))
	cp $@ $(temp_file)
	kernel2minor -k $(temp_file) -r $(temp_file).new $(1)
	mv $(temp_file).new $@
	rm -f $(temp_file)
endef

define Build/kernel-bin
	rm -f $@
	cp $< $@
endef

define Build/linksys-image
	$(TOPDIR)/scripts/linksys-image.sh \
		"$(call param_get_default,type,$(1),$(DEVICE_NAME))" \
		$@ $@.new
		mv $@.new $@
endef

define Build/lzma
	$(call Build/lzma-no-dict,-lc1 -lp2 -pb2 $(1))
endef

define Build/lzma-no-dict
	$(STAGING_DIR_HOST)/bin/lzma e $@ $(1) $@.new
	@mv $@.new $@
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
		-B $(NETGEAR_BOARD_ID) -v $(VERSION_DIST).$(firstword $(subst -, ,$(REVISION))) \
		$(if $(NETGEAR_HW_ID),-H $(NETGEAR_HW_ID)) \
		-r "$(1)" \
		-i $@ -o $@.new
	mv $@.new $@
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

define Build/pad-extra
	dd if=/dev/zero bs=$(1) count=1 >> $@
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

define Build/pad-rootfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $@ $(1) \
		$(if $(BLOCKSIZE),$(BLOCKSIZE:%k=%),4 8 16 64 128 256)
endef

define Build/pad-to
	$(call Image/pad-to,$@,$(1))
endef

define Build/patch-cmdline
	$(STAGING_DIR_HOST)/bin/patch-cmdline $@ '$(CMDLINE)'
endef

# Convert a raw image into a $1 type image.
# E.g. | qemu-image vdi
define Build/qemu-image
	if command -v qemu-img; then \
		qemu-img convert -f raw -O $1 $@ $@.new; \
		mv $@.new $@; \
	else \
		echo "WARNING: Install qemu-img to create VDI/VMDK images" >&2; exit 1; \
	fi
endef

define Build/qsdk-ipq-factory-nand
	$(TOPDIR)/scripts/mkits-qsdk-ipq-image.sh \
		$@.its ubi $@
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/qsdk-ipq-factory-nor
	$(TOPDIR)/scripts/mkits-qsdk-ipq-image.sh \
		$@.its hlos $(IMAGE_KERNEL) rootfs $(IMAGE_ROOTFS)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv $@.new $@
endef

define Build/seama
	$(STAGING_DIR_HOST)/bin/seama -i $@ \
		-m "dev=/dev/mtdblock/$(SEAMA_MTDBLOCK)" -m "type=firmware"
	mv $@.seama $@
endef

define Build/seama-seal
	$(STAGING_DIR_HOST)/bin/seama -i $@ -s $@.seama \
		-m "signature=$(SEAMA_SIGNATURE)"
	mv $@.seama $@
endef

define Build/senao-header
	$(STAGING_DIR_HOST)/bin/mksenaofw $(1) -e $@ -o $@.new
	mv $@.new $@
endef

define Build/sysupgrade-tar
	sh $(TOPDIR)/scripts/sysupgrade-tar.sh \
		--board $(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)) \
		--kernel $(call param_get_default,kernel,$(1),$(IMAGE_KERNEL)) \
		--rootfs $(call param_get_default,rootfs,$(1),$(IMAGE_ROOTFS)) \
		$@
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

define Build/tplink-v1-header
	$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-c -H $(TPLINK_HWID) -W $(TPLINK_HWREV) -L $(KERNEL_LOADADDR) \
		-E $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-m $(TPLINK_HEADER_VERSION) -N "$(VERSION_DIST)" -V $(REVISION) \
		-k $@ -o $@.new $(1)
	@mv $@.new $@
endef

# combine kernel and rootfs into one image
# mktplinkfw <type> <optional extra arguments to mktplinkfw binary>
# <type> is "sysupgrade" or "factory"
#
# -a align the rootfs start on an <align> bytes boundary
# -j add jffs2 end-of-filesystem markers
# -s strip padding from end of the image
# -X reserve <size> bytes in the firmware image (hexval prefixed with 0x)
define Build/tplink-v1-image
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) \
		-N "$(VERSION_DIST)" -V $(REVISION) -m $(TPLINK_HEADER_VERSION) \
		-k $(IMAGE_KERNEL) -r $(IMAGE_ROOTFS) -o $@.new -j -X 0x40000 \
		-a $(call rootfs_align,$(FILESYSTEM)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-s) && mv $@.new $@ || rm -f $@
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

define Build/uImage
	mkimage \
		-A $(LINUX_KARCH) \
		-O linux \
		-T kernel \
		-C $(word 1,$(1)) \
		-a $(KERNEL_LOADADDR) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(if $(UIMAGE_NAME),$(UIMAGE_NAME),$(call toupper,$(LINUX_KARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION))' \
		$(if $(UIMAGE_MAGIC),-M $(UIMAGE_MAGIC)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		-d $@ $@.new
	mv $@.new $@
endef

define Build/xor-image
	$(STAGING_DIR_HOST)/bin/xorimage -i $@ -o $@.xor $(1)
	mv $@.xor $@
endef

define Build/zip
	mkdir $@.tmp
	mv $@ $@.tmp/$(1)

	zip -j -X \
		$(if $(SOURCE_DATE_EPOCH),--mtime="$(SOURCE_DATE_EPOCH)") \
		$@ $@.tmp/$(if $(1),$(1),$@)
	rm -rf $@.tmp
endef

define Build/zyxel-ras-image
	let \
		newsize="$(subst k,* 1024,$(RAS_ROOTFS_SIZE))"; \
		$(STAGING_DIR_HOST)/bin/mkrasimage \
			-b $(RAS_BOARD) \
			-v $(RAS_VERSION) \
			-r $@ \
			-s $$newsize \
			-o $@.new \
			$(if $(findstring separate-kernel,$(word 1,$(1))),-k $(IMAGE_KERNEL)) \
		&& mv $@.new $@
endef
