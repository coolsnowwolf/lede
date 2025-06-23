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
        H3C_DEVICE_ID \
        H3C_PRODUCT_ID \
	LINKSYS_HEADER \
        ZYXEL_VERS

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

define Build/linksys-image
	$(SCRIPT_DIR)/linksys-image.sh $(@) $(@).new $(LINKSYS_MODEL)
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
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
