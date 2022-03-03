# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2021 OpenWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=grub
PKG_CPE_ID:=cpe:/a:gnu:grub2
PKG_VERSION:=2.06
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/grub
PKG_HASH:=b79ea44af91b93d17cd3fe80bdae6ed43770678a9a5ae192ccea803ebb657ee1

HOST_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=grub2/host

ifneq ($(BUILD_VARIANT),none)
  PKG_ASLR_PIE:=0
  PKG_SSP:=0
endif

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/grub2/Default
  CATEGORY:=Boot Loaders
  SECTION:=boot
  TITLE:=GRand Unified Bootloader ($(1))
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86
  VARIANT:=$(1)
endef

Package/grub2=$(call Package/grub2/Default,pc)
Package/grub2-efi=$(call Package/grub2/Default,efi)

define Package/grub2-editenv
  CATEGORY:=Utilities
  SECTION:=utils
  SUBMENU:=Boot Loaders
  TITLE:=Grub2 Environment editor
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86
  VARIANT:=none
endef

define Package/grub2-editenv/description
	Edit grub2 environment files.
endef

define Package/grub2-bios-setup
  CATEGORY:=Utilities
  SECTION:=utils
  SUBMENU:=Boot Loaders
  TITLE:=Grub2 BIOS boot setup tool
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86
  VARIANT:=none
endef

define Package/grub2-bios-setup/description
	Set up images to bootable.
endef

HOST_BUILD_PREFIX := $(STAGING_DIR_HOST)

CONFIGURE_VARS += \
	grub_build_mkfont_excuse="don't want fonts"

CONFIGURE_ARGS += \
	--target=$(REAL_GNU_TARGET_NAME) \
	--disable-werror \
	--disable-nls \
	--disable-device-mapper \
	--disable-libzfs \
	--disable-liblzma \
	--disable-grub-mkfont \
	--with-platform=$(BUILD_VARIANT)

HOST_CONFIGURE_VARS += \
	grub_build_mkfont_excuse="don't want fonts"

HOST_CONFIGURE_ARGS += \
	--disable-grub-mkfont \
	--target=$(REAL_GNU_TARGET_NAME) \
	--sbindir="$(STAGING_DIR_HOST)/bin" \
	--disable-werror \
	--disable-libzfs \
	--disable-nls \
	--with-platform=none

HOST_MAKE_FLAGS += \
	TARGET_RANLIB=$(TARGET_RANLIB) \
	LIBLZMA=$(STAGING_DIR_HOST)/lib/liblzma.a


ifneq ($(BUILD_VARIANT),none)
  TARGET_CFLAGS := $(filter-out -O2 -O3 -fno-plt,$(TARGET_CFLAGS))
  MAKE_PATH := grub-core
endif

define Host/Configure
	$(SED) 's,(RANLIB),(TARGET_RANLIB),' $(HOST_BUILD_DIR)/grub-core/Makefile.in
	$(Host/Configure/Default)
endef

define Package/grub2/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)/grub2
	$(CP) $(PKG_BUILD_DIR)/grub-core/boot.img $(STAGING_DIR_IMAGE)/grub2/
	$(CP) $(PKG_BUILD_DIR)/grub-core/cdboot.img $(STAGING_DIR_IMAGE)/grub2/
	sed 's#msdos1#gpt1#g' ./files/grub-early.cfg >$(PKG_BUILD_DIR)/grub-early.cfg
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O i386-pc \
		-c $(PKG_BUILD_DIR)/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/gpt-core.img \
		at_keyboard biosdisk boot chain configfile fat linux ls part_gpt reboot serial vga
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O i386-pc \
		-c ./files/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/generic-core.img \
		at_keyboard biosdisk boot chain configfile ext2 linux ls part_msdos reboot serial vga
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O i386-pc \
		-c ./files/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/eltorito.img \
		at_keyboard biosdisk boot chain configfile iso9660 linux ls part_msdos reboot serial test vga
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O i386-pc \
		-c ./files/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/legacy-core.img \
		biosdisk boot chain configfile ext2 linux ls part_msdos reboot serial vga
endef

define Package/grub2-efi/install
	sed 's#msdos1#gpt1#g' ./files/grub-early.cfg >$(PKG_BUILD_DIR)/grub-early.cfg
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O $(CONFIG_ARCH)-efi \
		-c $(PKG_BUILD_DIR)/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/boot$(if $(CONFIG_x86_64),x64,ia32).efi \
		at_keyboard boot chain configfile fat linux ls part_gpt reboot serial efi_gop efi_uga
	$(STAGING_DIR_HOST)/bin/grub-mkimage \
		-d $(PKG_BUILD_DIR)/grub-core \
		-p /boot/grub \
		-O $(CONFIG_ARCH)-efi \
		-c ./files/grub-early.cfg \
		-o $(STAGING_DIR_IMAGE)/grub2/iso-boot$(if $(CONFIG_x86_64),x64,ia32).efi \
		at_keyboard boot chain configfile fat iso9660 linux ls part_msdos part_gpt reboot serial test efi_gop efi_uga
endef

define Package/grub2-editenv/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/grub-editenv $(1)/usr/sbin/
endef

define Package/grub2-bios-setup/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/grub-bios-setup $(1)/usr/sbin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,grub2))
$(eval $(call BuildPackage,grub2-efi))
$(eval $(call BuildPackage,grub2-editenv))
$(eval $(call BuildPackage,grub2-bios-setup))
