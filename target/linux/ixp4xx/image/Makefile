# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ifdef CONFIG_PACKAGE_apex
  define Image/Build/Linksys
	BIN_DIR=$(BIN_DIR) $(TOPDIR)/scripts/slugimage.pl \
		-L $(STAGING_DIR_IMAGE)/apex-$(2)-armeb.bin \
		-k $(BIN_DIR)/$(IMG_PREFIX)-$(2)-zImage \
		-r rootfs:$(BIN_DIR)/$(IMG_PREFIX)-$(1).img \
		-p -o $(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1).bin
	BIN_DIR=$(BIN_DIR) $(TOPDIR)/scripts/slugimage.pl \
		-F -L $(STAGING_DIR_IMAGE)/apex-$(2)-16mb-armeb.bin \
		-k $(BIN_DIR)/$(IMG_PREFIX)-$(2)-zImage \
		-r rootfs:$(BIN_DIR)/$(IMG_PREFIX)-$(1).img \
		-p -o $(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1)-16mb.bin
  endef
endif

define Image/Build/Freecom
	$(INSTALL_DIR) $(TARGET_DIR)/boot
	# TODO: Add special CMDLINE shim for webupgrade image here
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-$(2)-zImage $(TARGET_DIR)/zImage
	$(TAR) cfj $(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1).img --numeric-owner --owner=0 --group=0 -C $(TARGET_DIR)/ .
	$(STAGING_DIR_HOST)/bin/encode_crc $(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1).img $(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(1)-webupgrade.img
	rm -f $(TARGET_DIR)/zImage
endef

define Image/BuildKernel
	cp $(KDIR)/zImage $(BIN_DIR)/$(IMG_PREFIX)-zImage
	BIN_DIR=$(BIN_DIR) IMG_PREFIX="$(IMG_PREFIX)" $(TOPDIR)/scripts/arm-magic.sh
endef

define Image/BuildKernel/Initramfs
	cp $(KDIR)/zImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-initramfs-zImage
	BIN_DIR=$(BIN_DIR) IMG_PREFIX="$(IMG_PREFIX)-initramfs" $(TOPDIR)/scripts/arm-magic.sh
endef

# Build sysupgrade image
define BuildFirmware/Generic
	dd if=$(KDIR)/zImage of=$(KDIR)/zImage.pad bs=64k conv=sync; \
	dd if=$(KDIR)/root.$(1) of=$(KDIR)/root.$(1).pad bs=128k conv=sync; \
	sh $(TOPDIR)/scripts/combined-image.sh \
		$(KDIR)/zImage.pad \
		$(KDIR)/root.$(1).pad \
		$(BIN_DIR)/$(IMG_PREFIX)-$(patsubst jffs2-%,jffs2,$(patsubst squashfs-%,squashfs,$(1)))-sysupgrade.bin
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	$(call BuildFirmware/Generic,$(1))
endef

define Image/Build/jffs2-64k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=65536 conv=sync
endef

define Image/Build/jffs2-128k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=131072 conv=sync
	$(call Image/Build/Linksys,$(1),nslu2,$(1))
	$(call Image/Build/Freecom,$(1),fsg3,$(1))
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=131072 conv=sync
	$(call Image/Build/Linksys,$(1),nslu2,$(1))
	$(call Image/Build/Freecom,$(1),fsg3,$(1))
endef

$(eval $(call BuildImage))
