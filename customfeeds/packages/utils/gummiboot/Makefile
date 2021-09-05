#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gummiboot
PKG_VERSION:=45
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rzr/gummiboot.git
PKG_SOURCE_DATE:=2021-04-11
PKG_SOURCE_VERSION:=eb3daf2ca4cb1657cf1f780957485d690a552bf6
PKG_MIRROR_HASH:=4c57791693b57bbe36e85b49d70310728b8008c4c545006a71c5a5f71b8df501

PKG_LICENSE:=LGPL-2.1-or-later
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_DEPENDS:=gnu-efi
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/gummiboot
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=Simple UEFI boot manager
  DEPENDS:=@TARGET_x86_64 +libblkid
  URL:=https://github.com/rzr/gummiboot
endef

define Package/gummiboot/description
  gummiboot Simple UEFI boot manager

  gummiboot executes EFI images. The default entry is selected by a configured
  pattern (glob) or an on-screen menu.
endef

CONFIGURE_ARGS += \
	--with-efi-libdir=$(STAGING_DIR)/usr/lib \
	--with-efi-ldsdir=$(STAGING_DIR)/usr/lib \
	--with-efi-includedir=$(STAGING_DIR)/usr/include

define Build/Compile
	+$(MAKE_VARS) EFI_CFLAGS="-I$(TOOLCHAIN_DIR)/include $(TARGET_CFLAGS)" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_FLAGS) \
		$(1);
endef

define Build/Install
	$(MAKE_VARS) EFI_CFLAGS="-I$(TOOLCHAIN_DIR)/include $(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_INSTALL_FLAGS) install
endef

define Package/gummiboot/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/lib/gummiboot
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/gummiboot/gummibootx64.efi $(1)/usr/lib/gummiboot/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gummiboot $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,gummiboot))
