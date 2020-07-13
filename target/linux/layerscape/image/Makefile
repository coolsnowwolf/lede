#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

LS_SD_ROOTFSPART_OFFSET = 64
LS_SD_IMAGE_SIZE = $(shell echo $$((($(LS_SD_ROOTFSPART_OFFSET) + \
	$(CONFIG_TARGET_ROOTFS_PARTSIZE)) * 1024 * 1024)))

define Build/ls-clean
	rm -f $@
endef

define Build/ls-append
	dd if=$(STAGING_DIR_IMAGE)/$(1) >> $@
endef

define Build/ls-append-dtb
	$(call Image/BuildDTB,$(DTS_DIR)/$(1).dts,$(DTS_DIR)/$(1).dtb)
	dd if=$(DTS_DIR)/$(1).dtb >> $@
endef

define Build/ls-append-sdhead
	./gen_sdcard_head_img.sh $(STAGING_DIR_IMAGE)/$(1)-sdcard-head.img \
		$(LS_SD_ROOTFSPART_OFFSET) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
	dd if=$(STAGING_DIR_IMAGE)/$(1)-sdcard-head.img >> $@
endef

define Build/traverse-fit
	./mkits-multiple-config.sh -o $@.its -A $(LINUX_KARCH) \
		-v $(LINUX_VERSION) -k $@ -a $(KERNEL_LOADADDR) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-C gzip -c 1 -c 2 \
		-d $(DEVICE_DTS_DIR)/freescale/traverse-ls1043s.dtb \
		-D "Traverse_LS1043S" -n "ls1043s" -a $(FDT_LOADADDR) -c 1 \
		-d $(DEVICE_DTS_DIR)/freescale/traverse-ls1043v.dtb \
		-D "Traverse_LS1043V" -n "ls1043v" -a $(FDT_LOADADDR) -c 2
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $@.its $@.new
	@mv -f $@.new $@
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
