include $(TOPDIR)/rules.mk

PKG_NAME:=antfs-mount
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/antfs-mount
	SECTION:=utils
	CATEGORY:=Utilities
	SUBMENU:=Filesystem
	TITLE:=NTFS mount script for AVM NTFS driver
	PKGARCH:=all
	DEPENDS:=+kmod-fs-antfs
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/antfs-mount/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/mount.ntfs $(1)/sbin
endef

$(eval $(call BuildPackage,antfs-mount))
