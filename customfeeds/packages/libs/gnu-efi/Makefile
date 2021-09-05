#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gnu-efi
PKG_VERSION:=3.0.9
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/vathpela/gnu-efi.git
PKG_SOURCE_DATE:=2021-04-11
PKG_SOURCE_VERSION:=3e4d5c79905afcd815b0beb3dcfe2dfae5b3e6dd
PKG_MIRROR_HASH:=7660d2259c1d5208bcabee5a0ffb6dc61f41363a79ba9158f3dd413a8af8e238
PKG_BUILD_PARALLEL:=1

PKG_MAINTAINER:=Oskari Rauta <oskari.rauta@gmail.com>
PKG_LICENSE_FILES:=README.efilib

include $(INCLUDE_DIR)/package.mk

define Package/gnu-efi
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=GNU's EFI library
  URL:=https://github.com/vathpela/gnu-efi
  HIDDEN:=1
  DEPENDS:=@TARGET_x86_64
endef

define Package/gnu-efi/description
  GNU's EFI library
endef

define Build/Install
	$(MAKE_VARS) \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_INSTALL_FLAGS) \
		INSTALLROOT=$(PKG_INSTALL_DIR) \
		install
endef

define Package/gnu-efi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/local/lib/** $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/efi
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/local/lib/** $(1)/usr/lib/
	cp -aR $(PKG_INSTALL_DIR)/usr/local/include/efi/** $(1)/usr/include/efi/
endef

$(eval $(call BuildPackage,gnu-efi))
