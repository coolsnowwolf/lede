#
# Copyright (C) 2006-2014 OpenWrt.org
# Copyright 2010 Vertical Communications
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=e2fsprogs
PKG_VERSION:=1.44.3
PKG_HASH:=5d899f7d30f481cc0c6a049ebe26ebe145f1b524182ea1ecde4086162d4e4bb6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/kernel/people/tytso/e2fsprogs/v$(PKG_VERSION)/
PKG_LICENSE:=GPL-2.0
PKG_CPE_ID:=cpe:/a:e2fsprogs_project:e2fsprogs

PKG_BUILD_DEPENDS:=util-linux e2fsprogs/host
PKG_INSTALL:=1

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/e2fsprogs
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=Ext2/3/4 filesystem utilities
  URL:=http://e2fsprogs.sourceforge.net/
  DEPENDS:=+libuuid +libext2fs
endef

define Package/e2fsprogs/description
 This package contains essential ext2 filesystem utilities which consists of
 e2fsck, mke2fs and most of the other core ext2 filesystem utilities.
endef

define Package/libext2fs
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=ext2/3/4 filesystem library
  URL:=http://e2fsprogs.sourceforge.net/
  DEPENDS:=+libuuid +libblkid +libss +libcomerr
endef

define Package/libext2fs/description
 libext2fs is a library which can access ext2, ext3 and ext4 filesystems.
endef

define Package/libss
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=command-line interface parsing library
  URL:=http://e2fsprogs.sourceforge.net/
  DEPENDS:=+libcomerr
endef

define Package/libss/description
  This pacakge contains libss, a command-line interface parsing library
  bundled with e2fsprogs.
endef

define Package/libcomerr
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=common error description library
  URL:=http://e2fsprogs.sourceforge.net/
  DEPENDS:=+libuuid
endef

define Package/libcomerr/description
  This package contains libcom_err, the common error description library
  bundled with e2fsprogs.
endef

define Package/tune2fs
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem tune utility
  DEPENDS:= +e2fsprogs
endef

define Package/resize2fs
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem resize utility
  DEPENDS:= +e2fsprogs
endef

define Package/badblocks
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem badblocks utility
  DEPENDS:= +e2fsprogs
endef

define Package/dumpe2fs
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem information dumping utility
  DEPENDS:= +e2fsprogs
endef

define Package/e2freefrag
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem free space fragmentation information utility
  DEPENDS:= +e2fsprogs
endef

define Package/filefrag
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem file fragmentation report utility
  DEPENDS:= +e2fsprogs
endef

define Package/debugfs
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem debugger
  DEPENDS:= +e2fsprogs
endef

define Package/chattr
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem chattr utility
  DEPENDS:= +e2fsprogs
endef

define Package/lsattr
$(call Package/e2fsprogs)
  TITLE:=Ext2 Filesystem lsattr utility
  DEPENDS:= +e2fsprogs
endef

TARGET_CFLAGS += $(FPIC) -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -flto

CONFIGURE_ARGS += \
	--disable-testio-debug \
	--enable-elf-shlibs	\
	--disable-libuuid	\
	--disable-libblkid	\
	--disable-uuidd		\
	--disable-tls		\
	--disable-nls		\
	--disable-rpath		\
	--disable-fuse2fs

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(PKG_BUILD_DIR)/config/
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/util \
		BUILDCC="$(HOSTCC)" \
		CFLAGS="" \
		CPPFLAGS="" \
		LDFLAGS="" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1,) \
		subst
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		LDFLAGS=-Wl,--gc-sections \
		BUILDCC="$(HOSTCC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		ELF_OTHER_LIBS="$(TARGET_LDFLAGS) -luuid" \
		SYSLIBS="$(TARGET_LDFLAGS) -ldl -L$(PKG_BUILD_DIR)/lib/ -l:libcom_err.so.0.0" \
		V=$(if $(findstring c,$(OPENWRT_VERBOSE)),1,) \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/lib/ext2fs/ext2fs.pc $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_BUILD_DIR)/lib/et/com_err.pc $(1)/usr/lib/pkgconfig

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libext2fs.{so,a}* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libcom_err.{so,a}* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/libss.{so,a}* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/include/ext2fs
	$(CP) $(PKG_BUILD_DIR)/lib/ext2fs/*.h $(1)/usr/include/ext2fs
	$(INSTALL_DIR) $(1)/usr/include/et
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/et/*.h $(1)/usr/include/et
	# Apparently there is some confusion
	echo "#include <et/com_err.h>" > $(1)/usr/include/com_err.h
	$(INSTALL_DIR) $(1)/usr/include/ss
	$(CP) \
		$(PKG_BUILD_DIR)/lib/ss/ss.h \
		$(PKG_BUILD_DIR)/lib/ss/ss_err.h \
		$(1)/usr/include/ss/
endef

define Host/Compile
	$(MAKE) $(PKG_JOBS) -C $(HOST_BUILD_DIR)/lib/ss mk_cmds
	$(MAKE) $(PKG_JOBS) -C $(HOST_BUILD_DIR)/lib/et compile_et
endef

define Host/Install
	$(INSTALL_DIR) $(1)/share/et
	$(CP) $(HOST_BUILD_DIR)/lib/et/et_[ch].awk $(1)/share/et/
	$(INSTALL_DIR) $(1)/share/ss
	$(CP) $(HOST_BUILD_DIR)/lib/ss/ct_c.{sed,awk} $(1)/share/ss/
	$(INSTALL_DIR) $(1)/bin
	$(CP) \
		$(HOST_BUILD_DIR)/lib/et/compile_et \
		$(HOST_BUILD_DIR)/lib/ss/mk_cmds \
		$(1)/bin/
endef

define Package/e2fsprogs/conffiles
/etc/e2fsck.conf
endef

define Package/e2fsprogs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/e2fsck $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/mke2fs $(1)/usr/sbin/
	$(LN) mke2fs $(1)/usr/sbin/mkfs.ext2
	$(LN) mke2fs $(1)/usr/sbin/mkfs.ext3
	$(LN) mke2fs $(1)/usr/sbin/mkfs.ext4
	$(LN) e2fsck $(1)/usr/sbin/fsck.ext2
	$(LN) e2fsck $(1)/usr/sbin/fsck.ext3
	$(LN) e2fsck $(1)/usr/sbin/fsck.ext4
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libe2p.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/lib/functions/fsck
	$(INSTALL_DATA) ./files/e2fsck.sh $(1)/lib/functions/fsck/
	$(INSTALL_DATA) ./files/e2fsck.conf $(1)/etc/e2fsck.conf
endef

define Package/libcomerr/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcom_err.so* $(1)/usr/lib/
endef

define Package/libss/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libss.so* $(1)/usr/lib/
endef

define Package/libext2fs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libext2fs.so.* $(1)/usr/lib/
endef

define Package/libext2fs/install_lib
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/ext2fs/libext2fs.a $(1)/usr/lib/libext2fs_pic.a
endef

define Package/tune2fs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/tune2fs $(1)/usr/sbin/
	$(LN) tune2fs $(1)/usr/sbin/findfs
endef

define Package/resize2fs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/resize2fs $(1)/usr/sbin/
endef

define Package/badblocks/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/badblocks $(1)/usr/sbin/
endef

define Package/dumpe2fs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/dumpe2fs $(1)/usr/sbin/
endef

define Package/e2freefrag/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/e2freefrag $(1)/usr/sbin/
endef

define Package/filefrag/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/filefrag $(1)/usr/sbin/
endef

define Package/debugfs/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/debugfs $(1)/usr/sbin/
endef

define Package/chattr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/chattr $(1)/usr/bin/
endef

define Package/lsattr/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lsattr $(1)/usr/bin/
endef

$(eval $(call BuildPackage,e2fsprogs))
$(eval $(call BuildPackage,libcomerr))
$(eval $(call BuildPackage,libss))
$(eval $(call BuildPackage,libext2fs))
$(eval $(call BuildPackage,tune2fs))
$(eval $(call BuildPackage,resize2fs))
$(eval $(call BuildPackage,badblocks))
$(eval $(call BuildPackage,dumpe2fs))
$(eval $(call BuildPackage,e2freefrag))
$(eval $(call BuildPackage,filefrag))
$(eval $(call BuildPackage,debugfs))
$(eval $(call BuildPackage,chattr))
$(eval $(call BuildPackage,lsattr))
$(eval $(call HostBuild))
