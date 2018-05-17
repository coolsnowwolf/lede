DEVICE_VARS += TPLINK_HWID TPLINK_HWREV TPLINK_FLASHLAYOUT TPLINK_HEADER_VERSION TPLINK_BOARD_NAME

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

# add RE450 and similar header to the kernel image
define Build/mktplinkfw-kernel
	$(STAGING_DIR_HOST)/bin/mktplinkfw-kernel \
		-H $(TPLINK_HWID) -N OpenWrt -V $(REVISION) \
		-L $(KERNEL_LOADADDR) -E $(KERNEL_LOADADDR) \
		-k $@ \
		-o $@.new
	@mv $@.new $@
endef

define Build/uImageArcher
	mkimage -A $(LINUX_KARCH) \
		-O linux -T kernel \
		-C $(1) -a $(KERNEL_LOADADDR) -e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-n '$(call toupper,$(LINUX_KARCH)) LEDE Linux-$(LINUX_VERSION)' -d $@ $@.new
	@mv $@.new $@
endef


define Device/tplink
  TPLINK_HWREV := 0x1
  TPLINK_HEADER_VERSION := 1
  LOADER_TYPE := gz
  KERNEL := kernel-bin | patch-cmdline | lzma
  KERNEL_INITRAMFS := kernel-bin | patch-cmdline | lzma | mktplinkfw-combined
#  IMAGES := sysupgrade.bin
  IMAGES := sysupgrade.bin factory.bin
  IMAGE/sysupgrade.bin := append-rootfs | mktplinkfw sysupgrade
#  IMAGE/factory.bin := append-rootfs | mktplinkfw factory | a
endef

define Device/tplink-nolzma
$(Device/tplink)
  LOADER_FLASH_OFFS := 0x22000
  COMPILE := loader-$(1).gz
  COMPILE/loader-$(1).gz := loader-okli-compile
  #KERNEL := copy-file $(KDIR)/vmlinux.bin.lzma | uImage lzma -M 0x4f4b4c49 | loader-okli $(1)
  KERNEL:= kernel-bin | append-dtb | lzma |uImage lzma -M 0x4f4b4c49 | loader-okli $(1)
  KERNEL_INITRAMFS := copy-file $(KDIR)/vmlinux-initramfs.bin.lzma | loader-kernel-cmdline | mktplinkfw-combined
endef

define Device/tplink-4m
$(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 4M
  IMAGE_SIZE := 3904k
endef

define Device/tplink-8m
$(Device/tplink-nolzma)
  TPLINK_FLASHLAYOUT := 8M
  IMAGE_SIZE := 7936k
endef

define Device/tplink-4mlzma
$(Device/tplink)
  TPLINK_FLASHLAYOUT := 4Mlzma
  IMAGE_SIZE := 3904k
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

define Device/tl_wr1043nd_v1
  $(Device/tplink-8m)
  ATH_SOC := ar9132
  DEVICE_TITLE := TP-LINK TL-WR1043N/ND v1
  DEVICE_PACKAGES := kmod-usb-core kmod-usb2 kmod-usb-ledtrig-usbport
  BOARDNAME := TL-WR1043ND
  DEVICE_PROFILE := TLWR1043
  TPLINK_HWID := 0x10430001
endef

#TARGET_DEVICES += tl_wr1043nd_v1
