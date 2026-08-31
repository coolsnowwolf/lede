# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

SQUASHFSCOMP := xz $(LZMA_XZ_OPTIONS)
LS_SD_KERNELPART_SIZE = 24
LS_SD_KERNELPART_OFFSET = 3
LS_SD_ROOTFSPART_OFFSET = 32
LS_SD_IMAGE_SIZE = $(shell echo $$((($(LS_SD_ROOTFSPART_OFFSET) + \
	$(CONFIG_TARGET_ROOTFS_PARTSIZE)))))

define Build/sdcard-img
        rm -fR $@.boot
        mkdir -p $@.boot
        $(CP) $(KDIR)/$(DEVICE_NAME)$(KERNEL_SUFFIX) $@.boot
        $(if $(DEVICE_DTS),\
                $(foreach dtb,$(DEVICE_DTS),$(CP) $(KDIR)/image-$(dtb).dtb $@.boot), \
                $(CP) $(KDIR)/image-/*.dtb $@.boot)

        PADDING=1 $(SCRIPT_DIR)/gen_image_generic.sh \
                $@ \
                $(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
                $(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
                2048

        $(if $(UBOOT),dd if=$(STAGING_DIR_IMAGE)/$(UBOOT).img of=$@ bs=512 skip=1 seek=1 conv=notrunc)
endef

define Build/ls-clean
	rm -f $@
endef

define Build/ls-append
	dd if=$(STAGING_DIR_IMAGE)/$(1) >> $@
endef

define Build/ls-append-dtb
	dd if=$(DTS_DIR)/$(1).dtb >> $@
endef

define Build/ls-append-kernel
	mkdir -p $@.tmp && \
	cp $(IMAGE_KERNEL) $@.tmp/fitImage && \
	make_ext4fs -J -L kernel -l "$(LS_SD_KERNELPART_SIZE)M" \
		$(if $(SOURCE_DATE_EPOCH),-T $(SOURCE_DATE_EPOCH)) \
		"$@.kernel.part" "$@.tmp" && \
	dd if=$@.kernel.part >> $@ && \
	rm -rf $@.tmp && \
	rm -f $@.kernel.part
endef

define Build/ls-append-sdhead
	./gen_sdcard_head_img.sh $(STAGING_DIR_IMAGE)/$(1)-sdcard-head.img \
		$(LS_SD_KERNELPART_OFFSET) $(LS_SD_KERNELPART_SIZE) \
		$(LS_SD_ROOTFSPART_OFFSET) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
	dd if=$(STAGING_DIR_IMAGE)/$(1)-sdcard-head.img >> $@
endef

define Device/Default
  PROFILES := Default
  DEVICE_DTS := $(subst _,-,$(1))
  KERNEL_DEPENDS = $$(wildcard $(DTS_DIR)/$$(DEVICE_DTS).dts)
  KERNEL_ENTRY := 0x00000000
  KERNEL_LOADADDR := 0x00000000
  KERNEL := kernel-bin
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
