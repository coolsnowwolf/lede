include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80060000

DEVICE_VARS += LOADER_FLASH_OFFS LOADER_TYPE
DEVICE_VARS += LOADER_FLASH_MAX LOADER_KERNEL_MAGIC
DEVICE_VARS += NETGEAR_BOARD_ID NETGEAR_HW_ID
DEVICE_VARS += RAS_BOARD RAS_ROOTFS_SIZE RAS_VERSION

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
	$(call Build/loader-common,LOADER_DATA="$@")
endef

define Build/loader-okli-compile
	$(call Build/loader-common, \
		FLASH_OFFS=$(LOADER_FLASH_OFFS) \
		FLASH_MAX=$(LOADER_FLASH_MAX) \
		KERNEL_MAGIC=$(LOADER_KERNEL_MAGIC) )
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
	append-rootfs | pad-rootfs | append-metadata | check-size
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
