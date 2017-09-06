#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

JFFS2_BLOCKSIZE = 128k

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

TARGET_DTBS := rbt-832 dlan-usb-extender

DTBS_DIR:=dts/

LOADADDR:=0x00008000

UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage

define Image/Build/MkuImage
	mkimage -A arm -O linux -T kernel -a $(LOADADDR) -C none -e $(LOADADDR) \
	-n 'ARM LEDE Linux-$(LINUX_VERSION)' -d $(1) $(2);
endef

define Image/Build/DTB
	cp $(KDIR)/zImage$(2) $(KDIR)/zImage-$(1);
	cat $(LINUX_DIR)/arch/$(ARCH)/boot/$(DTBS_DIR)$(1).dtb >> $(KDIR)/zImage$(2)-$(1);
	$(call Image/Build/MkuImage,$(KDIR)/zImage$(2)-$(1),$(KDIR)/uImage$(2)-$(1))
	cp $(KDIR)/uImage$(2)-$(1) $(UIMAGE)$(2)-$(1);
endef

define Image/Build/Profile/dLAN_USB_Extender
	dd if=$(KDIR)/uImage-dlan-usb-extender bs=64k conv=sync of=$(KDIR)/$(IMG_PREFIX)-dlan-usb-extender-upgrade-$(1).bin
	cat $(KDIR)/root.$(1) >> $(KDIR)/$(IMG_PREFIX)-dlan-usb-extender-upgrade-$(1).bin
	cp $(KDIR)/$(IMG_PREFIX)-dlan-usb-extender-upgrade-$(1).bin $(BIN_DIR)/
endef

define Image/BuildKernel
	$(foreach dtb,$(TARGET_DTBS),$(call Image/Build/DTB,$(dtb)))
endef

define Image/Build/Initramfs
	$(foreach dtb,$(TARGET_DTBS),$(call Image/Build/DTB,$(dtb),-initramfs))
endef

define Image/Build/squashfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(KDIR)/root.squashfs 128
endef

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	$(call Image/Build/Profile/$(PROFILE),$(1))
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(call Image/Build/Initramfs)
endif
endef

$(eval $(call BuildImage))
