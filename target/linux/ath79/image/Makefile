include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80060000

DEVICE_VARS += CMDLINE CONSOLE IMAGE_SIZE BOARDNAME LOADER_FLASH_OFFS LOADER_TYPE ATH_SOC

define Build/loader-common
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" LOADER_NAME="$(notdir $@)" \
		BOARD="$(BOARDNAME)" \
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		$(1) compile loader.$(LOADER_TYPE)
	mv "$@.$(LOADER_TYPE)" "$@"
	rm -rf $@.src
endef

define Build/loader-kernel
	$(call Build/loader-common,LOADER_DATA="$@")
endef

define Build/loader-kernel-cmdline
	$(call Build/loader-common,LOADER_DATA="$@" KERNEL_CMDLINE="$(CMDLINE)")
endef


define Build/loader-okli-compile
	$(call Build/loader-common,FLASH_OFFS=$(LOADER_FLASH_OFFS) FLASH_MAX=0 KERNEL_CMDLINE="$(CMDLINE)")
endef

define Build/loader-okli
	dd if=$(KDIR)/loader-$(1).gz bs=7680 conv=sync of="$@.new"
	cat "$@" >> "$@.new"
	mv "$@.new" "$@"
endef

define Build/relocate-kernel
	rm -rf $@.relocate
	$(CP) ../../generic/image/relocate $@.relocate
	$(MAKE) -j1 -C $@.relocate KERNEL_ADDR=$(KERNEL_LOADADDR) CROSS_COMPILE=$(TARGET_CROSS)
	( \
		dd if=$@.relocate/loader.bin bs=32 conv=sync && \
		perl -e '@s = stat("$@"); print pack("N", @s[7])' && \
		cat "$@" \
	) > "$@.new"
	mv "$@.new" "$@"
	rm -rf $@.relocate
endef

define Build/copy-file
	cat "$(1)" > "$@"
endef

define Device/Default
  ATH_SOC :=
  BOARDNAME :=
  DEVICE_PROFILE = $$(BOARDNAME)
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(ATH_SOC)_$(1)
  PROFILES = Default Minimal $$(DEVICE_PROFILE)
  MTDPARTS :=
  BLOCKSIZE := 64k
  CONSOLE = ttyS0,115200
  CMDLINE = $$(if $$(BOARDNAME),board=$$(BOARDNAME)) $$(if $$(MTDPARTS),mtdparts=$$(MTDPARTS)) $$(if $$(CONSOLE),console=$$(CONSOLE))
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  COMPILE :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | append-rootfs | pad-rootfs
endef

ifeq ($(SUBTARGET),generic)
include ./tp-link.mk
include ./generic.mk
include ./generic-ubnt.mk
endif
$(eval $(call BuildImage))
