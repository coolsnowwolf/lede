#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xfsprogs
PKG_VERSION:=5.9.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/fs/xfs/xfsprogs
PKG_HASH:=bc5c805596bc609a18dc1f1b4ed6a2821dba9f47408ec00e7799ceea1b2097f1

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSES/GPL-2.0
PKG_CPE_ID:=cpe:/a:sgi:xfsprogs

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/xfsprogs/default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  DEPENDS:=+libuuid +libpthread
  URL:=https://xfs.org/
endef

define Package/xfs-admin
$(call Package/xfsprogs/default)
  TITLE:=Utilities for changing parameters of an XFS filesystems
endef

define Package/xfs-mkfs
$(call Package/xfsprogs/default)
  TITLE:=Utility for creating XFS filesystems
endef

define Package/xfs-fsck
$(call Package/xfsprogs/default)
  TITLE:=Utilities for checking and repairing XFS filesystems
endef

define Package/xfs-growfs
$(call Package/xfsprogs/default)
  TITLE:=Utility for increasing the size of XFS filesystems
endef

CONFIGURE_ARGS += \
	--disable-gettext \
	--disable-blkid \
	--disable-readline \
	--disable-editline \
	--disable-termcap \
	--disable-lib64 \
	--disable-librt \
	--disable-ubisan \
	--disable-addrsan \
	--disable-threadsan \
	--disable-scrub \
	--disable-libicu

TARGET_CFLAGS += -DHAVE_MAP_SYNC
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)

define Package/xfs-admin/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/xfs_db $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/xfs_admin $(1)/sbin
endef

define Package/xfs-mkfs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/mkfs.xfs $(1)/usr/sbin
endef

define Package/xfs-fsck/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/xfs_repair $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/xfs_db $(1)/usr/sbin
endef

define Package/xfs-growfs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/xfs_growfs $(1)/usr/sbin
endef

$(eval $(call BuildPackage,xfs-admin))
$(eval $(call BuildPackage,xfs-mkfs))
$(eval $(call BuildPackage,xfs-fsck))
$(eval $(call BuildPackage,xfs-growfs))
