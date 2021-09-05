#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntfs-3g
PKG_VERSION:=2017.3.23
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)_ntfsprogs-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://www.tuxera.com/opensource/
PKG_HASH:=3e5a021d7b761261836dcb305370af299793eedbded731df3d6943802e1262d5

PKG_MAINTAINER:=Ted Hess <thess@kitschensync.net>
PKG_LICENSE:=GPL-2.0-only LGPL-2.1-or-later
PKG_LICENSE_FILES:=COPYING COPYING.LIB
PKG_CPE_ID:=cpe:/a:ntfs-3g:ntfs-3g

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

# release contains fuseext/int hint
PKG_RELEASE:=$(PKG_RELEASE)$(if $(CONFIG_PACKAGE_NTFS-3G_USE_LIBFUSE),-fuseext,-fuseint)

# define build dir, respect fuseext/int
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/ntfs-3g/common
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://www.tuxera.com
  SUBMENU:=Filesystem
  TITLE:=Stable Read/Write NTFS Driver
endef

define Package/ntfs-3g
  $(call Package/ntfs-3g/common)
  DEPENDS+= +kmod-fuse +PACKAGE_NTFS-3G_USE_LIBFUSE:libfuse +libpthread
endef

define Package/ntfs-3g/description
  Ntfs-3g is a NTFS driver, which can create, remove, rename,
  move files, directories, hard links, and streams. It can read
  and write files, including streams and sparse files. It can
  handle special files like symbolic links, devices, and FIFOs.
  Moreover it can also read transparently compressed files.

  Contains:
   - ntfs-3g
   - ntfs-3g.probe
   - mount.ntfs-3g (symlink to ntfs-3g)

endef

define Package/ntfs-3g/config
config PACKAGE_NTFS-3G_USE_LIBFUSE
	bool "use external FUSE library, selects package libfuse"
	depends on PACKAGE_ntfs-3g
	---help---
	Ntfs-3g by default uses a minimalized lite version of FUSE.
	If libfuse is part of your filesystem anyway (because of sshfs, owfs
	etc.) it makes sense to activate this option and save some kilobytes
	of space.

config PACKAGE_NTFS-3G_HAS_PROBE
	bool "install the ntfs-3g.probe utility"
	depends on PACKAGE_ntfs-3g
	default y
endef

define Package/ntfs-3g-low
  $(call Package/ntfs-3g/common)
  TITLE:=lowntfs-3g (alternative using the fuse low-level interface)
  DEPENDS+= +ntfs-3g
endef

define Package/ntfs-3g-low/description
  Contains:
   - lowntfs-3g
   - mount.lowntfs-3g (symlink to lowntfs-3g)

  A driver variant using the fuse low-level interface missing some of the
  enhanced functionality for streams or the like. You might want to check:
  http://www.tuxera.com/community/ntfs-3g-manual/

endef


define Package/ntfs-3g-utils/description
  Suite of NTFS utilities for doing neat things with NTFS.
  Contains:
   - mkntfs - Create an NTFS filesystem.
   - ntfscat - Dump a file's content to the standard output.
   - ntfsclone - Efficiently clone, backup, restore or rescue NTFS.
   - ntfscluster - Locate the files which use the given sectors or clusters.
   - ntfscmp - Compare two NTFS filesystems and tell the differences.
   - ntfscp - Copy a file to an NTFS volume.
   - ntfssecaudit - Display ownership and permissions, check consistency
   - ntfsfix - Check and fix some common errors, clear the LogFile.
   - ntfsinfo - Show information about NTFS or one of the files or directories within it.
   - ntfslabel - Show, or set, an NTFS filesystem's volume label.
   - ntfsls - List information about files in a directory residing on an NTFS.
   - ntfsresize - Resize NTFS without losing data.
   - ntfsundelete - Recover deleted files from NTFS.
   - ntfsusermap - Define mapping of Windows accounts to Linux logins
endef

define Package/ntfs-3g-utils
  $(call Package/ntfs-3g/common)
  TITLE:=ntfs-3g utilities
  DEPENDS+= +ntfs-3g +libgcrypt +libuuid
endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--with-uuid

# configure/make according selection
ifdef CONFIG_PACKAGE_NTFS-3G_USE_LIBFUSE
	CONFIGURE_ARGS += --with-fuse=external
	TARGET_CPPFLAGS:=-I$(STAGING_DIR)/usr/include/fuse $(TARGET_CPPFLAGS)
else
	CONFIGURE_ARGS += --with-fuse=internal
	TARGET_CPPFLAGS:=-I../include/fuse-lite $(TARGET_CPPFLAGS)
endif

# enable ntfsprogs and extras
ifneq ($(CONFIG_PACKAGE_ntfs-3g-utils)$(SDK)$(DEVELOPER),)
	CONFIGURE_ARGS += --enable-ntfsprogs --enable-extras
else
	CONFIGURE_ARGS += --disable-ntfsprogs --disable-extras
endif

# redefine prepare to extract to our build dir
# apply patches
define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/
	$(TAR) -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip 1
	$(Build/Patch)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/ntfs-3g $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/libntfs-3g.{la,a,so*} $(1)/usr/lib/
endef

define Package/ntfs-3g/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ntfs-3g $(1)/usr/bin/
	$(if $(CONFIG_PACKAGE_NTFS-3G_HAS_PROBE),$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ntfs-3g.probe $(1)/usr/bin/,)
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libntfs-3g.so.* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/sbin
	$(LN) ../usr/bin/ntfs-3g $(1)/sbin/mount.ntfs-3g
endef

define Package/ntfs-3g/postinst
#!/bin/sh
FILE="$${IPKG_INSTROOT}/etc/filesystems"
ID="ntfs-3g"

if ! [ -f '/etc/filesystems' ]; then
	echo "Create '$$FILE'."
	touch "$$FILE"
fi

if ! grep -q -e '^ntfs-3g$$' "$$FILE"; then
	echo "Add '$$ID' to known filesystems."
	echo "$$ID" >> "$$FILE"
fi

endef

define Package/ntfs-3g-low/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lowntfs-3g $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/sbin
	$(LN) ../usr/bin/lowntfs-3g $(1)/sbin/mount.lowntfs-3g
endef

define Package/ntfs-3g-low/postinst
#!/bin/sh
FILE="$${IPKG_INSTROOT}/etc/filesystems"
ID="lowntfs-3g"

if ! [ -f '/etc/filesystems' ]; then
	echo "Create '$$FILE'."
	touch "$$FILE"
fi

if ! grep -q -e '^ntfs-3g$$' "$$FILE"; then
	echo "Add '$$ID' to known filesystems."
	echo "$$ID" >> "$$FILE"
fi

endef

define Package/ntfs-3g-utils/install
	$(INSTALL_DIR) $(1)/sbin
	$(LN) ../usr/sbin/mkntfs $(1)/sbin/mkfs.ntfs
	$(INSTALL_DIR) $(1)/usr/bin
	$(FIND) $(PKG_INSTALL_DIR)/usr/bin/ -type f ! -regex '.*[^/]*ntfs-3g[^/]*' -exec $(INSTALL_BIN) {} $(1)/usr/bin/ \;
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,ntfs-3g))
$(eval $(call BuildPackage,ntfs-3g-low))
$(eval $(call BuildPackage,ntfs-3g-utils))
