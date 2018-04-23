#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Image/Prepare
	mkimage -A arm -O linux -T kernel -C none -a 0x20008000 -e 0x20008000 -n '$(VERSION_DIST) Linux-$(LINUX_VERSION)' -d $(KDIR)/zImage $(KDIR)/uImage
endef

LEGACY_NOR_KERNEL_SIZE = 2048k
LEGACY_SPI_KERNEL_SIZE = 1536k

# Build sysupgrade image
define BuildFirmware/Legacy
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-$(1)-sysupgrade.bin \
		of=$(KDIR)/uImage.pad bs=$(3) count=1
	dd if=$(BIN_DIR)/$(IMG_PREFIX)-$(1)-sysupgrade.bin \
		of=$(KDIR)/root.$(1).pad bs=$(3) skip=1
	sh $(TOPDIR)/scripts/combined-image.sh \
		$(KDIR)/uImage.pad \
		$(KDIR)/root.$(1).pad \
		$(BIN_DIR)/$(IMG_PREFIX)-$(1)-$(2)-legacy-sysupgrade.bin
endef

define Image/BuildKernel/Initramfs
	mkimage -A arm -O linux -T kernel -C none -a 0x20008000 -e 0x20008000 -n '$(VERSION_DIST) Linux-$(LINUX_VERSION)' -d $(KDIR)/zImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-initramfs-uImage
endef

define Image/Build
	cat $(KDIR)/uImage $(KDIR)/root.$(1) > $(BIN_DIR)/$(IMG_PREFIX)-$(1)-sysupgrade.bin
	$(call Image/Build/$(1),$(1))
	$(call BuildFirmware/Legacy,$(1),laguna_spi,$(LEGACY_SPI_KERNEL_SIZE))
	$(call BuildFirmware/Legacy,$(1),laguna_nor,$(LEGACY_NOR_KERNEL_SIZE))
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(BIN_DIR)/$(IMG_PREFIX)-$(1)-sysupgrade.bin)
endef

$(eval $(call BuildImage))
