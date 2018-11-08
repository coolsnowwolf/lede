include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=grub

include ../common.mk

PKG_BUILD_DEPENDS:=grub2/host

define Package/grub2
$(call Package/grub2/Default)
endef

define Package/grub2-editenv
  CATEGORY:=Utilities
  SECTION:=utils
  TITLE:=Grub2 Environment editor
  URL:=http://www.gnu.org/software/grub/
  DEPENDS:=@TARGET_x86||TARGET_x86_64
endef

define Package/grub2-editenv/description
	Edit grub2 environment files.
endef

define Package/grub2-editenv/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/grub-editenv $(1)/usr/sbin/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,grub2))
$(eval $(call BuildPackage,grub2-editenv))
