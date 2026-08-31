# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80100000

DEVICE_VARS += \
	BELKIN_HEADER \
	BELKIN_MODEL \
        CAMEO_BOARD_MODEL \
        CAMEO_BOARD_VERSION \
        CAMEO_CUSTOMER_SIGNATURE \
        CAMEO_KERNEL_PART \
        CAMEO_KERNEL_PART_SIZE \
        CAMEO_ROOTFS_PART \
	ECS_BOARD_ID \
        FLASH_ADDR \
        H3C_DEVICE_ID \
        H3C_PRODUCT_ID \
	LINKSYS_HEADER \
	ZYNFW_BOARD \
	ZYNFW_ALIGN \
	ZYXEL_VERS

define Build/rt-compress
	$(STAGING_DIR_HOST)/bin/xz -9 --format=lzma --stdout "$@" > "$@.new"
	mv "$@.new" "$@"
endef

define Build/rt-loader
	$(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
		KERNEL_IMG_IN="$@" KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
	mv "$@.new" "$@"
endef

define Build/rt-loader-no-uimage
        $(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
                KERNEL_ADDR="$(KERNEL_LOADADDR)" KERNEL_IMG_IN="$@" \
                KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
        mv "$@.new" "$@"
endef

define Build/rt-loader-standalone
	$(MAKE) all clean -C rt-loader CROSS_COMPILE="$(TARGET_CROSS)" \
		FLASH_ADDR=$(FLASH_ADDR) KERNEL_IMG_OUT="$@.new" BUILD_DIR="$@.build"
	mv "$@.new" "$@"
endef

define Build/zyxel-vers
       ( echo VERS;\
       for hw in $(ZYXEL_VERS); do\
               echo -n "V9.99($$hw.0) | ";\
               date -d @$(SOURCE_DATE_EPOCH) +%m/%d/%Y;\
       done ) >> $@
endef

define Build/cameo-tag
	$(SCRIPT_DIR)/cameo-tag.py $@ $(CAMEO_KERNEL_PART_SIZE)
endef

define Build/cameo-version
	echo -n "OpenWrt" >> $@
	dd if=/dev/zero bs=25 count=1 >> $@
endef

define Build/cameo-headers
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) count=1 of=$@.kernel_part; \
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) skip=1 of=$@.rootfs_part; \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.kernel_part $@.kernel_part.hex \
                "$(CAMEO_BOARD_MODEL)" os $(CAMEO_KERNEL_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION) \
                $(KERNEL_LOADADDR); \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.rootfs_part $@.rootfs_part.hex \
                "$(CAMEO_BOARD_MODEL)" squashfs $(CAMEO_ROOTFS_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION); \
        cat $@.kernel_part.hex $@.rootfs_part.hex > $@
endef

define Build/7z
	$(STAGING_DIR_HOST)/bin/7zr a $(@).new -t7z -m0=lzma $(@)
	mv $@.new $@
endef

define Build/h3c-image
	$(STAGING_DIR_HOST)/bin/mkh3cimg \
		-i $(@) \
		-o $(@).new \
		-c 7z \
		-p $(H3C_PRODUCT_ID) \
		-d $(H3C_DEVICE_ID)
	mv $@.new $@
endef

define Build/h3c-vfs
	$(STAGING_DIR_HOST)/bin/mkh3cvfs \
		-i $(@) \
		-o $(@).new \
		-f openwrt-kernel.bin
	mv $@.new $@
endef

define Build/belkin-header
	$(SCRIPT_DIR)/belkin-header.py $(@) $(@).new $(BELKIN_HEADER) ${BELKIN_MODEL}
	mv $@.new $@
endef

# Edgecore images use a 100-byte uImage header. The last two bytes contain
# the board ID stored in the hwinfo partition.
define Build/edgecore-header
	$(STAGING_DIR_HOST)/bin/uimage_padhdr -i $@ -o $@.new -l 36 -s 0:1.0.0.0 -x 34:$(ECS_BOARD_ID)
	mv $@.new $@
endef

define Build/linksys-image
	$(SCRIPT_DIR)/linksys-image.sh $(@) $(@).new $(LINKSYS_MODEL)
	mv $@.new $@
endef

define Build/xikestor-nosimg
	$(STAGING_DIR_HOST)/bin/nosimg-enc -i $@ -o $@.new
	mv $@.new $@
endef

define Build/xikestor-sks7300-img
	$(STAGING_DIR_HOST)/bin/sks7300-img \
		-i $@ \
		-o $@.new \
		-n OpenWrt \
		-v $(VERSION_DIST)$(firstword $(subst +, , $(firstword $(subst -, ,$(REVISION)))))
	mv $@.new $@
endef

define Build/zynsig
	$(STAGING_DIR_HOST)/bin/zynsig -i $@ \
		-o $@.new
	mv $@.new $@
endef

define Device/Default
  PROFILES = Default
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  COMPILE :=
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata
endef

define Device/kernel-lzma
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	lzma | \
	uImage lzma
endef

define Device/uimage-rt-loader
	KERNEL/rt-loader := kernel-bin | append-dtb | rt-compress | rt-loader
	KERNEL := $$(KERNEL/rt-loader) | uImage none
	KERNEL_INITRAMFS := $$(KERNEL/rt-loader) | uImage none
endef

define Device/rt-loader-bootbase
	KERNEL/rt-compress := kernel-bin | append-dtb | rt-compress
	KERNEL := $$(KERNEL/rt-compress) | uImage lzma
	KERNEL_INITRAMFS := $$(KERNEL/rt-compress) | uImage lzma | rt-loader
endef

define Build/zynos-firmware
	$(STAGING_DIR_HOST)/bin/mkzynfw \
	    -B $(ZYNFW_BOARD) \
	    -b $(KDIR)/loader-$(DEVICE_NAME).bin \
	    -r $@:$(ZYNFW_ALIGN) \
	    -o $@.new
	mv $@.new $@
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
