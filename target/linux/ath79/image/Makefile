include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80060000

DEVICE_VARS += LOADER_FLASH_OFFS LOADER_TYPE
DEVICE_VARS += LOADER_FLASH_MAX LOADER_KERNEL_MAGIC
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
DEVICE_VARS += RAS_BOARD RAS_ROOTFS_SIZE RAS_VERSION
DEVICE_VARS += SEAMA_SIGNATURE SEAMA_MTDBLOCK

define Build/combined-image
	sh $(TOPDIR)/scripts/combined-image.sh \
		"$(IMAGE_KERNEL)" \
		"$@" \
		"$@.new"
	@mv $@.new $@
endef

define Build/loader-common
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" LOADER_NAME="$(notdir $@)" \
		$(1) compile loader.$(LOADER_TYPE)
	mv "$@.$(LOADER_TYPE)" "$@"
	rm -rf $@.src
endef

define Build/loader-kernel
	$(call Build/loader-common, \
		LOADER_DATA="$@" \
		BOARD="$(DEVICE_NAME)" )
endef

define Build/loader-okli-compile
	$(call Build/loader-common, \
		FLASH_OFFS=$(LOADER_FLASH_OFFS) \
		FLASH_MAX=$(LOADER_FLASH_MAX) \
		KERNEL_MAGIC=$(LOADER_KERNEL_MAGIC) \
		BOARD="$(DEVICE_NAME)" )
endef

# Arguments: <output name> <kernel offset>
define Build/loader-okli
	dd if=$(KDIR)/loader-$(word 1,$(1)).$(LOADER_TYPE) bs=$(word 2,$(1)) conv=sync of="$@.new"
	cat "$@" >> "$@.new"
	mv "$@.new" "$@"
endef

define Build/append-loader-okli
	cat "$(KDIR)/loader-$(word 1,$(1)).$(LOADER_TYPE)" >> "$@"
endef

define Build/append-loader-okli-uimage
	cat "$(KDIR)/loader-$(word 1,$(1)).uImage" >> "$@"
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


define Device/Default
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  PROFILES = Default
  MTDPARTS :=
  BLOCKSIZE := 64k
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | uImage lzma
  LOADER_KERNEL_MAGIC :=
  LOADER_FLASH_MAX :=
  LOADER_FLASH_OFFS :=
  LOADER_TYPE :=
  COMPILE :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin = append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size | append-metadata
endef

define Device/loader-okli-uimage
  LOADER_TYPE := bin
  COMPILE := loader-$(1).bin loader-$(1).uImage
  COMPILE/loader-$(1).bin := loader-okli-compile
  COMPILE/loader-$(1).uImage := append-loader-okli $(1) | pad-to 64k | \
	lzma | uImage lzma
endef

define Device/seama
  KERNEL := kernel-bin | append-dtb | relocate-kernel | lzma
  KERNEL_INITRAMFS := $$(KERNEL) | seama
  IMAGES += factory.bin
  SEAMA_MTDBLOCK := 1

  # 64 bytes offset:
  # - 28 bytes seama_header
  # - 36 bytes of META data (4-bytes aligned)
  IMAGE/default := append-kernel | pad-offset $$$$(BLOCKSIZE) 64 | append-rootfs
  IMAGE/sysupgrade.bin := $$(IMAGE/default) | seama | pad-rootfs | \
	check-size | append-metadata
  IMAGE/factory.bin := $$(IMAGE/default) | pad-rootfs -x 64 | seama | \
	seama-seal | check-size
  SEAMA_SIGNATURE :=
endef

include $(SUBTARGET).mk

ifeq ($(SUBTARGET),generic)
include generic-tp-link.mk
include generic-ubnt.mk
endif

ifeq ($(SUBTARGET),tiny)
include tiny-netgear.mk
include tiny-tp-link.mk
endif

$(eval $(call BuildImage))
