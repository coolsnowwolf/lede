DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION TPLINK_BOARD_NAME TPLINK_BOARD_ID

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs%,0x4,$(patsubst root.%,%,$(1))))))
endef

# combine kernel and rootfs into one image
# mktplinkfw <type> <optional extra arguments to mktplinkfw binary>
# <type> is "sysupgrade" or "factory"
#
# -a align the rootfs start on an <align> bytes boundary
# -j add jffs2 end-of-filesystem markers
# -s strip padding from end of the image
# -X reserve <size> bytes in the firmware image (hexval prefixed with 0x)
define Build/mktplinkfw
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) -N OpenWrt -V $(REVISION) \
		-m $(TPLINK_HEADER_VERSION) \
		-k $(IMAGE_KERNEL) \
		-r $@ \
		-o $@.new \
		-j -X 0x40000 \
		-a $(call rootfs_align,$(FILESYSTEM)) \
		$(wordlist 2,$(words $(1)),$(1)) \
		$(if $(findstring sysupgrade,$(word 1,$(1))),-s) && mv $@.new $@ || rm -f $@
endef

# mktplinkfw-combined
#
# -c combined image
define Build/mktplinkfw-combined
	$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(TPLINK_HWID) -W $(TPLINK_HWREV) -F $(TPLINK_FLASHLAYOUT) -N OpenWrt -V $(REVISION) $(1) \
		-m $(TPLINK_HEADER_VERSION) \
		-k $@ \
		-o $@.new \
		-s -S \
		-c
	@mv $@.new $@
endef

define Build/uImageArcher
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(call toupper,$(LINUX_KARCH)) OpenWrt Linux-$(LINUX_VERSION)' -d $@ $@.new
	@mv $@.new $@
endef

define Device/tplink
  TPLINK_HWREV := 0x1
  TPLINK_HEADER_VERSION := 1
  LOADER_TYPE := gz
  KERNEL := kernel-bin | append-dtb | lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | tplink-v1-header
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade | append-metadata
  IMAGE/factory.bin := append-rootfs | mktplinkfw factory
endef

define Device/tplink-nolzma
  $(Device/tplink)
  LOADER_FLASH_OFFS := 0x22000
  COMPILE := loader-$(1).gz
  COMPILE/loader-$(1).gz := loader-okli-compile
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma -M 0x4f4b4c49 | loader-okli $(1) 7680
  KERNEL_INITRAMFS := kernel-bin | append-dtb | gzip | tplink-v1-header
endef

define Device/tplink-4m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 4M
  IMAGE_SIZE := 3904k
endef

define Device/tplink-4mlzma
  $(Device/tplink)
  TPLINK_FLASHLAYOUT := 4Mlzma
  IMAGE_SIZE := 3904k
endef

define Device/tplink-8m
  $(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 8M
  IMAGE_SIZE := 7936k
endef

define Device/tplink-8mlzma
$(Device/tplink)
  TPLINK_FLASHLAYOUT := 8Mlzma
  IMAGE_SIZE := 7936k
endef

define Device/tplink-16mlzma
  $(Device/tplink)
  TPLINK_FLASHLAYOUT := 16Mlzma
  IMAGE_SIZE := 15872k
endef

define Device/tplink-safeloader
  $(Device/tplink)
  KERNEL := kernel-bin | append-dtb | lzma | tplink-v1-header
  IMAGE/sysupgrade.bin := append-rootfs | tplink-safeloader sysupgrade | append-metadata
  IMAGE/factory.bin := append-rootfs | tplink-safeloader factory
endef

define Device/tplink-safeloader-uimage
  $(Device/tplink-safeloader)
  KERNEL := kernel-bin | append-dtb | lzma | uImageArcher lzma
endef
