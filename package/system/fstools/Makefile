#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fstools
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/fstools.git
PKG_SOURCE_DATE:=2018-12-28
PKG_SOURCE_VERSION:=af93f4b8dc32cf1320dfe8f4b93bb3a12606fc33
PKG_MIRROR_HASH:=a2d8c2500fe75014ef16c039d8cdb276c1d5c066f8bce6550a3ecf46a9a411a9
CMAKE_INSTALL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_USE_MIPS16:=0
PKG_FLAGS:=nonshared

PKG_BUILD_DEPENDS := util-linux
PKG_CONFIG_DEPENDS := CONFIG_NAND_SUPPORT CONFIG_FSTOOLS_UBIFS_EXTROOT

PKG_MAINTAINER:=John Crispin <john@phrozen.org>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)
CMAKE_OPTIONS += $(if $(CONFIG_FSTOOLS_UBIFS_EXTROOT),-DCMAKE_UBIFS_EXTROOT=y)
CMAKE_OPTIONS += $(if $(CONFIG_FSTOOLS_OVL_MOUNT_FULL_ACCESS_TIME),-DCMAKE_OVL_MOUNT_FULL_ACCESS_TIME=y)
CMAKE_OPTIONS += $(if $(CONFIG_FSTOOLS_OVL_MOUNT_COMPRESS_ZLIB),-DCMAKE_OVL_MOUNT_COMPRESS_ZLIB=y)

define Package/fstools
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+ubox +USE_GLIBC:librt +NAND_SUPPORT:ubi-utils
  TITLE:=OpenWrt filesystem tools
  MENU:=1
endef

define Package/fstools/config
	config FSTOOLS_UBIFS_EXTROOT
		depends on PACKAGE_fstools
		depends on NAND_SUPPORT
		bool "Support extroot functionality with UBIFS"
		default y
		help
			This option makes it possible to use extroot functionality if the root filesystem resides on an UBIFS partition

	config FSTOOLS_OVL_MOUNT_FULL_ACCESS_TIME
		depends on PACKAGE_fstools
		bool "Full access time accounting"
		default n
		help
			This option enables the full access time accounting (warning: it will increase the flash writes).

	config FSTOOLS_OVL_MOUNT_COMPRESS_ZLIB
		depends on PACKAGE_fstools
		bool "Compress using zlib"
		default n
		help
			This option enables the compression using zlib on the storage device.
endef

define Package/snapshot-tool
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=rootfs snapshoting tool
  DEPENDS:=+libubox +fstools
endef

define Package/block-mount
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Block device mounting and checking
  DEPENDS:=+ubox +libubox +libuci
endef

define Package/blockd
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Block device automounting
  DEPENDS:=+block-mount +fstools +libubus +kmod-fs-autofs4
endef

define Package/fstools/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{mount_root,jffs2reset} $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libfstools.so $(1)/lib/
	$(LN) jffs2reset $(1)/sbin/jffs2mark
endef

define Package/snapshot-tool/install
	$(INSTALL_DIR) $(1)/sbin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/snapshot_tool $(1)/sbin/
	$(INSTALL_BIN) ./files/snapshot $(1)/sbin/
endef

define Package/block-mount/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib $(1)/usr/sbin $(1)/etc/hotplug.d/block $(1)/etc/init.d/ $(1)/etc/uci-defaults/

	$(INSTALL_BIN) ./files/fstab.init $(1)/etc/init.d/fstab
	$(INSTALL_CONF) ./files/fstab.default $(1)/etc/uci-defaults/10-fstab
	$(INSTALL_CONF) ./files/mount.hotplug $(1)/etc/hotplug.d/block/10-mount

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/block $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libblkid-tiny.so $(1)/lib/
	$(LN) ../../sbin/block $(1)/usr/sbin/swapon
	$(LN) ../../sbin/block $(1)/usr/sbin/swapoff

endef

define Package/blockd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/blockd $(1)/sbin/
	$(INSTALL_BIN) ./files/blockd.init $(1)/etc/init.d/blockd
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libubi-utils.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,fstools))
$(eval $(call BuildPackage,snapshot-tool))
$(eval $(call BuildPackage,block-mount))
$(eval $(call BuildPackage,blockd))
