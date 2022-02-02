include $(TOPDIR)/rules.mk

PKG_NAME:=shellsync
PKG_VERSION:=0.2
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/shellsync
  CATEGORY:=Utilities
  TITLE:=Sync shell scripts
  DEPENDS:=+libpthread +kmod-macvlan
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/shellsync/description
A tool to sync different shell scripts.
Based on syncppp patch by morfast.
endef

define Build/Compile
	$(TARGET_CROSS)gcc -pthread -o $(PKG_BUILD_DIR)/shellsync $(PKG_BUILD_DIR)/shellsync.c
endef

define Package/shellsync/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/shellsync $(1)/usr/bin
endef

$(eval $(call BuildPackage,shellsync))
