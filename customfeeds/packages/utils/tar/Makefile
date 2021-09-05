#
# Copyright (C) 2008-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tar
PKG_VERSION:=1.34
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/tar
PKG_HASH:=63bebd26879c5e1eea4352f0d03c991f966aeb3ddeb3c7445c902568d5411d28

PKG_MAINTAINER:=Álvaro Fernández Rojas <noltari@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:gnu:tar

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tar
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+PACKAGE_TAR_POSIX_ACL:libacl +PACKAGE_TAR_XATTR:libattr +PACKAGE_TAR_BZIP2:bzip2
  EXTRA_DEPENDS:=$(if $(CONFIG_PACKAGE_TAR_XZ),xz)
  TITLE:=GNU tar
  URL:=https://www.gnu.org/software/tar/
  MENU:=1
  ALTERNATIVES:=300:/bin/tar:/usr/libexec/tar-gnu
endef

define Package/tar/config
	if PACKAGE_tar
		config PACKAGE_TAR_POSIX_ACL
			bool "tar: Enable POSIX ACL support"
			default y if USE_FS_ACL_ATTR
			default n

		config PACKAGE_TAR_XATTR
			bool "tar: Enable extended attribute (xattr) support"
			default y if USE_FS_ACL_ATTR
			default n

		config PACKAGE_TAR_BZIP2
			bool "tar: Enable seamless bzip2 support"
			default y

		config PACKAGE_TAR_GZIP
			bool "tar: Enable seamless gzip support"
			default y

		config PACKAGE_TAR_XZ
			bool "tar: Enable seamless xz support"
			select PACKAGE_xz-utils
			select PACKAGE_xz
			default y

		config PACKAGE_TAR_ZSTD
			bool "tar: Enable seamless zstd support"
			select PACKAGE_libzstd
			default y
	endif
endef

define Package/tar/description
	Tar is a program for packaging a set of files as a
	single archive in tar format.
endef

CONFIGURE_ARGS += \
	$(if $(CONFIG_PACKAGE_TAR_POSIX_ACL),--with,--without)-posix-acls \
	$(if $(CONFIG_PACKAGE_TAR_XATTR),--with,--without)-xattrs \
	$(if $(CONFIG_PACKAGE_TAR_BZIP2),--with-bzip2=bzip2,--without-bzip2) \
	$(if $(CONFIG_PACKAGE_TAR_GZIP),--with-gzip=gzip,--without-gzip) \
	$(if $(CONFIG_PACKAGE_TAR_XZ),--with-xz=xz,--without-xz) \
	$(if $(CONFIG_PACKAGE_TAR_ZSTD),--with-zstd=zstd,--without-zstd) \
	--without-compress \
	--without-lzip \
	--without-lzma \
	--without-lzop \
	--without-selinux \
	--disable-rpath

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDLAGS)"

define Package/tar/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tar $(1)/usr/libexec/tar-gnu
endef

$(eval $(call BuildPackage,tar))
