#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Image/BuildKernelMachId
	echo -en "\x$(2)\x1c\xa0\xe3\x$(3)\x10\x81\xe3" > $(KDIR)/$(1)-zImage
	cat $(LINUX_DIR)/arch/arm/boot/zImage >> $(KDIR)/$(1)-zImage
	$(STAGING_DIR_HOST)/bin/mkimage -A arm -O linux -T kernel \
	-C none -a 0x00008000 -e 0x00008000 -n 'Linux-$(LINUX_VERSION)' \
	-d $(KDIR)/$(1)-zImage $(KDIR)/$(1)-uImage
	cp $(KDIR)/$(1)-uImage $(BIN_DIR)/openwrt-$(1)-uImage
endef

define Image/BuildKernel
	# Orion Kernel uImages
 # DT2: mach id 1514 (0x5EA)
	$(call Image/BuildKernelMachId,dt2,05,ea)
 # LaCie 2big Network: mach id 2342 (0x926)
	$(call Image/BuildKernelMachId,net2big,09,26)
endef

define Image/Build/Freecom
	# Orion Freecom Images
 # backup unwanted files
	rm -rf ${TMP_DIR}/$2_backup
	mkdir ${TMP_DIR}/$2_backup
	-mv $(TARGET_DIR)/{var,jffs,rom} ${TMP_DIR}/$2_backup
 # add extra files
	$(INSTALL_DIR) $(TARGET_DIR)/boot
	# TODO: Add special CMDLINE shim for webupgrade image here
	$(CP) $(KDIR)/dt2-uImage $(TARGET_DIR)/boot/uImage
	$(INSTALL_DIR) $(TARGET_DIR)/var
 # create image
	$(TAR) cfj $(BIN_DIR)/openwrt-$(2)-$(1).img --numeric-owner --owner=0 --group=0 -C $(TARGET_DIR)/ .
	$(STAGING_DIR_HOST)/bin/encode_crc $(BIN_DIR)/openwrt-$(2)-$(1).img $(BIN_DIR)/openwrt-$(2)-$(1)-webupgrade.img $(3)
 # remove extra files
	rm -rf $(TARGET_DIR)/{boot,var}
 # recover unwanted files
	-mv ${TMP_DIR}/$2_backup/* $(TARGET_DIR)/
	rm -rf ${TMP_DIR}/$2_backup
endef

define Image/Build
$(call Image/Build/$(1),$(1))
$(call Image/Build/Freecom,$(1),dt2,DT,$(1))
endef

define Image/Build/squashfs
$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	( \
		dd if=$(KDIR)/uImage bs=1024k conv=sync; \
		dd if=$(KDIR)/root.$(1) bs=128k conv=sync; \
	) > $(BIN_DIR)/$(IMG_PREFIX)-$(1).img
endef
