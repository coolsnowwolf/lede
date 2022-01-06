include $(TOPDIR)/rules.mk

PKG_NAME:=ntfs3-mount
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/ntfs3-mount
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=NTFS mount script for Paragon NTFS3 driver
  DEPENDS:=+!LINUX_5_15:kmod-fs-ntfs3-oot +LINUX_5_15:kmod-fs-ntfs3
  PKGARCH:=all
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ntfs3-mount/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/mount.ntfs $(1)/sbin
endef

$(eval $(call BuildPackage,ntfs3-mount))
