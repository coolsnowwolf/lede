# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

SQUASHFSCOMP := xz $(LZMA_XZ_OPTIONS)

define Build/sdcard-img
        rm -fR $@.boot
        mkdir -p $@.boot
        $(CP) $(KDIR)/$(DEVICE_NAME)-kernel.bin $@.boot
        $(if $(DEVICE_DTS),\
                $(foreach dtb,$(DEVICE_DTS),$(CP) $(KDIR)/image-$(dtb).dtb $@.boot), \
                $(CP) $(KDIR)/image-/*.dtb $@.boot)

        $(SCRIPT_DIR)/gen_image_generic.sh \
                $@ \
                $(CONFIG_TARGET_KERNEL_PARTSIZE) $@.boot \
                $(CONFIG_TARGET_ROOTFS_PARTSIZE) $(IMAGE_ROOTFS) \
                2048

        $(if $(UBOOT),dd if=$(STAGING_DIR_IMAGE)/$(UBOOT).img of=$@ bs=512 skip=1 seek=1 conv=notrunc)
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
