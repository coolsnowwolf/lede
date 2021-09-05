#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rsync
PKG_VERSION:=3.2.3
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://download.samba.org/pub/rsync/src
PKG_HASH:=becc3c504ceea499f4167a260040ccf4d9f2ef9499ad5683c179a697146ce50e

PKG_MAINTAINER:=Maxim Storchak <m.storchak@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:rsync:rsync

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/rsync
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=Fast remote file copy program (like rcp)
  DEPENDS:=+libpopt +zlib +RSYNC_xattr:libattr +RSYNC_acl:libacl +RSYNC_zstd:libzstd $(ICONV_DEPENDS)
  URL:=https://rsync.samba.org/
  MENU:=1
endef

define Package/rsync/config
	source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS += $(if $(CONFIG_IPV6),-DINET6,)

CONFIGURE_ARGS += \
	--without-included-popt \
	--without-included-zlib \
	--disable-debug \
	--disable-asm \
	--disable-lz4 \
	--disable-locale \
	--disable-md2man \
	--disable-openssl \
	--disable-simd \
	--disable-xxhash \
	--$(if $(CONFIG_BUILD_NLS),en,dis)able-iconv \
	--$(if $(CONFIG_BUILD_NLS),en,dis)able-iconv-open \
	--$(if $(CONFIG_RSYNC_zstd),en,dis)able-zstd \
	--$(if $(CONFIG_RSYNC_xattr),en,dis)able-xattr-support \
	--$(if $(CONFIG_RSYNC_acl),en,dis)able-acl-support \
	$(if $(CONFIG_IPV6),,--disable-ipv6)

define Package/rsyncd
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=Rsync daemon
  DEPENDS:=+rsync
  URL:=https://rsync.samba.org/
endef

define Package/rrsync
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
  TITLE:=Restricted rsync script
  DEPENDS:=+rsync +perlbase-file +perl @(PACKAGE_openssh-server||PACKAGE_openssh-server-pam)
  URL:=https://www.samba.org/ftp/unpacked/rsync/support/rrsync
endef

define Package/rsync/description
 rsync is a program that allows files to be copied to and from remote machines
 in much the same way as rcp. It has many more options than rcp, and uses the
 rsync remote-update protocol to greatly speed up file transfers when the
 destination file already exists.

 The rsync remote-update protocol allows rsync to transfer just the differences
 between two sets of files across the network link.
endef

define Package/rsync/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin/
endef

define Package/rsyncd/description
 rsyncd is a configuration file and initscript to utilize rsync as a daemon. It
 uses the same binary as rsync.
endef

define Package/rsyncd/conffiles
/etc/rsyncd.conf
endef

define Package/rsyncd/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/rsyncd.conf $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rsyncd.init $(1)/etc/init.d/rsyncd
endef

define Package/rrsync/description
  rrsync is a script which wraps around rsync to restrict its permission to a
  particular subdirectory via ~/.ssh/authorized_keys and/or to read-only
  or write-only mode
endef

define Package/rrsync/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/support/rrsync $(1)/usr/bin
endef

$(eval $(call BuildPackage,rsync))
$(eval $(call BuildPackage,rsyncd))
$(eval $(call BuildPackage,rrsync))
