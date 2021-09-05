#
# Copyright (C) 2008-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lftp
PKG_VERSION:=4.9.2
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://lftp.tech/ftp/ \
		https://mirror.csclub.uwaterloo.ca/gentoo-distfiles/distfiles/
PKG_HASH:=c517c4f4f9c39bd415d7313088a2b1e313b2d386867fe40b7692b83a20f0670d

PKG_MAINTAINER:=Federico Di Marco <fededim@gmail.com>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:lftp_project:lftp

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/lftp
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libncurses +libopenssl +libreadline +libstdcpp +libexpat +zlib
  TITLE:=a sophisticated file transfer program with command line interface.
  URL:=https://lftp.yar.ru/
endef

define Package/lftp/description
LFTP is a sophisticated file transfer program with command line interface. It supports FTP, HTTP, FISH, SFTP, HTTPS and FTPS protocols. GNU Readline library is used for input.
Every operation in lftp is reliable, that is any non-fatal error is handled and the operation is retried automatically. So if downloading breaks, it will be restarted from the point automatically. Even if ftp server does not support REST command, lftp will try to retrieve the file from the very beginning until the file is transferred completely. This is useful for dynamic-ip machines which change their IP addresses quite often, and for sites with very bad internet connectivity.

lftp has shell-like command syntax allowing you to launch several commands in parallel in background (&). It is also possible to group commands within () and execute them in background. All background jobs are executed in the same single process. You can bring a foreground job to background with ^Z (c-z) and back with command `wait' (or `fg' which is alias to `wait'). To list running jobs, use command `jobs'. Some commands allow redirecting their output (cat, ls, ...) to file or via pipe to external command. Commands can be executed conditionally based on termination status of previous command (&&, ||).

lftp has builtin mirror which can download or update a whole directory tree. There is also reverse mirror (mirror -R) which uploads or updates a directory tree on server.
endef

CONFIGURE_ARGS += \
	--without-libiconv-prefix \
	--without-libintl-prefix \
	--without-gnutls \
	--without-libidn2 \
	--without-libresolv \
	--with-openssl="$(STAGING_DIR)/usr" \
	--with-readline="$(STAGING_DIR)/usr" \
	--with-zlib="$(STAGING_DIR)/usr" \
	--disable-static

define Package/lftp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/lftp $(1)/usr/bin/
endef

$(eval $(call BuildPackage,lftp))
