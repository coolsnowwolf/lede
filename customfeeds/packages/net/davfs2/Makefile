#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=davfs2
PKG_VERSION:=1.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SAVANNAH/davfs2
PKG_HASH:=2e6b675685bd917772be60f0999ae098c80f79ff20dc131e8fec4547cbb63b50

PKG_MAINTAINER:=Federico Di Marco <fededim@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:davfs2:davfs2

PKG_FIXUP:=gettext-version
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/davfs2
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  DEPENDS=+libneon +kmod-fuse +libfuse $(ICONV_DEPENDS) $(INTL_DEPENDS)
  TITLE:=Mount a WebDAV resource as a regular file system.
  URL:=https://savannah.nongnu.org/projects/davfs2/
endef

define Package/davfs2/description
 Web Distributed Authoring and Versioning (WebDAV), an extension to the HTTP-protocol,
 allows authoring of resources on a remote web server.davfs2 provides the ability to
 access such resources like a typical filesystem, allowing for use by standard
 applications with no built-in support for WebDAV.

 davfs2 is designed to fully integrate into the filesystem semantics of Unix-like
 systems (mount, umount, etc.). davfs2 makes mounting by unprivileged users as easy
 and secure as possible.

 davfs2 does extensive caching to make the file system responsive, to avoid
 unnecessary network traffic and to prevent data loss, and to cope for slow or
 unreliable connections.

 davfs2 will work with most WebDAV servers needing little or no configuration.
endef

define Package/davfs2/conffiles
/etc/davfs2/davfs2.conf
endef

CONFIGURE_ARGS += \
	--with-neon="$(STAGING_DIR)/usr" \
	--disable-rpath

define Package/davfs2/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mount.davfs $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/umount.davfs $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/davfs2
	$(INSTALL_DATA) files/$(PKG_NAME).conf $(1)/etc/davfs2
endef

$(eval $(call BuildPackage,davfs2))
