#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=f2fs-tools
PKG_VERSION:=1.16.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/linux/kernel/git/jaegeuk/f2fs-tools.git/snapshot/
PKG_HASH:=208c7a07e95383fbd7b466b5681590789dcb41f41bf197369c41a95383b57c5e

PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:f2fs-tools_project:f2fs-tools

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/f2fs-tools/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  DEPENDS:=+libf2fs
  URL:=http://git.kernel.org/cgit/linux/kernel/git/jaegeuk/f2fs-tools.git
  VARIANT:=default
endef

define Package/f2fs-tools/SELinux
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  DEPENDS:=+libf2fs-selinux +libselinux
  URL:=http://git.kernel.org/cgit/linux/kernel/git/jaegeuk/f2fs-tools.git
  VARIANT:=selinux
endef

define Package/mkf2fs
  $(Package/f2fs-tools/Default)
  TITLE:=Utility for creating a Flash-Friendly File System (F2FS)
  CONFLICTS:=mkf2fs-selinux
endef

define Package/mkf2fs-selinux
  $(Package/f2fs-tools/SELinux)
  TITLE:=Utility for creating a Flash-Friendly File System (F2FS) with SELinux support
endef

define Package/f2fsck
  $(Package/f2fs-tools/Default)
  TITLE:=Utility for checking/repairing a Flash-Friendly File System (F2FS)
  CONFLICTS:=f2fsck-selinux
endef

define Package/f2fsck-selinux
  $(Package/f2fs-tools/SELinux)
  TITLE:=Utility for checking/repairing a Flash-Friendly File System (F2FS) with SELinux support
endef

define Package/f2fs-tools
  $(Package/f2fs-tools/Default)
  TITLE:=Tools for Flash-Friendly File System (F2FS)
  DEPENDS += +mkf2fs +f2fsck
  CONFLICTS:=f2fs-tools-selinux
endef

define Package/f2fs-tools-selinux
  $(Package/f2fs-tools/SELinux)
  TITLE:=Tools for Flash-Friendly File System (F2FS) with SELinux support
  DEPENDS += +mkf2fs-selinux +f2fsck-selinux
endef

define Package/libf2fs
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for Flash-Friendly File System (F2FS) tools
  DEPENDS:=+libuuid
  ABI_VERSION:=6
  CONFLICTS:=libf2fs-selinux
  VARIANT:=default
endef

define Package/libf2fs-selinux
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Library for Flash-Friendly File System (F2FS) tools with SELinux support
  DEPENDS:=+libuuid +libselinux
  ABI_VERSION:=6
  VARIANT:=selinux
endef

CONFIGURE_ARGS += \
	--disable-static \
	--without-blkid \
	--without-lzo2 \
	--without-lz4

ifneq ($(BUILD_VARIANT),selinux)
  CONFIGURE_ARGS += --without-selinux
endif

CONFIGURE_VARS += \
	ac_cv_file__git=no

ifneq ($(CONFIG_USE_MUSL),)
  CONFIGURE_VARS += ac_cv_func_lseek64=yes
endif

define Package/libf2fs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libf2fs.so.* $(1)/usr/lib/
endef

Package/libf2fs-selinux/install = $(Package/libf2fs/install)

define Package/mkf2fs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mkfs.f2fs $(1)/usr/sbin
endef

Package/mkf2fs-selinux/install = $(Package/mkf2fs/install)

define Package/f2fsck/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/fsck.f2fs $(1)/usr/sbin
	$(LN) ../sbin/fsck.f2fs $(1)/usr/sbin/defrag.f2fs
	$(LN) ../sbin/fsck.f2fs $(1)/usr/sbin/dump.f2fs
	$(LN) ../sbin/fsck.f2fs $(1)/usr/sbin/sload.f2fs
	$(LN) ../sbin/fsck.f2fs $(1)/usr/sbin/resize.f2fs
endef

Package/f2fsck-selinux/install = $(Package/f2fsck/install)

define Package/f2fs-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/fibmap.f2fs $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/parse.f2fs $(1)/usr/sbin
endef

Package/f2fs-tools-selinux/install = $(Package/f2fs-tools/install)

$(eval $(call BuildPackage,libf2fs))
$(eval $(call BuildPackage,libf2fs-selinux))
$(eval $(call BuildPackage,mkf2fs))
$(eval $(call BuildPackage,mkf2fs-selinux))
$(eval $(call BuildPackage,f2fsck))
$(eval $(call BuildPackage,f2fsck-selinux))
$(eval $(call BuildPackage,f2fs-tools))
$(eval $(call BuildPackage,f2fs-tools-selinux))
