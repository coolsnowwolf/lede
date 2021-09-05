#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dovecot
PKG_VERSION:=2.3.13
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://dovecot.org/releases/2.3
PKG_HASH:=a3f875b80ec11a452480690108660030978c94fa8e796ad6d943a874b496f1c4

PKG_MAINTAINER:=Lucian Cristian <lucian.cristian@gmail.com>
PKG_LICENSE:=LGPL-2.1-only MIT BSD-3-Clause
PKG_LICENSE_FILES:=COPYING COPYING.LGPL COPYING.MIT
PKG_CPE_ID:=cpe:/a:dovecot:dovecot

PKG_CONFIG_DEPENDS:= \
	CONFIG_DOVECOT_GSSAPI \
	CONFIG_DOVECOT_LDAP \
	CONFIG_DOVECOT_MYSQL \
	CONFIG_DOVECOT_PGSQL \
	CONFIG_DOVECOT_SQLITE \
	CONFIG_DOVECOT_ICU

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
# iconv is needed when compiling with MySQL support. iconv will also be used by
# dovecot itself.
include $(INCLUDE_DIR)/nls.mk

define Package/dovecot
  SECTION:=mail
  CATEGORY:=Mail
  TITLE:=An IMAP and POP3 daemon
  URL:=https://www.dovecot.org/
  DEPENDS:=+DOVECOT_GSSAPI:krb5-libs +DOVECOT_LDAP:libopenldap +DOVECOT_MYSQL:libmysqlclient +DOVECOT_PGSQL:libpq +DOVECOT_SQLITE:libsqlite3 +libopenssl +librt +zlib +libbz2 +libcap +DOVECOT_ICU:icu $(ICONV_DEPENDS)
  USERID:=dovecot=59:dovecot=59 dovenull=60:dovenull=60
  ABI_VERSION:=$(PKG_VERSION)
endef

define Package/dovecot/description
 Dovecot is a program which provides POP3 and IMAP services.
endef

define Package/dovecot/config
	source "$(SOURCE)/Config.in"
endef

define Package/dovecot/conffiles
/etc/dovecot/
endef

define Package/dovecot-utils
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+dovecot
  TITLE:=doveadm and dsync utilities for Dovecot
endef

CONFIGURE_ARGS += \
	--libexecdir=/usr/libexec \
	--without-libunwind \
	--without-pam \
	--with-notify=dnotify \
	--without-lzma \
	--without-lz4 \
	--without-sodium \
	--without-zstd \
	$(if $(CONFIG_DOVECOT_GSSAPI),--with-gssapi=yes,--with-gssapi=no) \
	$(if $(CONFIG_DOVECOT_LDAP),--with-ldap=yes,--with-ldap=no) \
	$(if $(CONFIG_DOVECOT_MYSQL),--with-mysql=yes,--with-mysql=no) \
	$(if $(CONFIG_DOVECOT_PGSQL),--with-pgsql=yes,--with-pgsql=no) \
	$(if $(CONFIG_DOVECOT_SQLITE),--with-sqlite=yes,--with-sqlite=no) \
	$(if $(CONFIG_DOVECOT_ICU),--with-icu=yes,--with-icu=no)

CONFIGURE_VARS += \
	RPCGEN= \
	i_cv_signed_size_t=no \
	i_cv_signed_time_t=no \
	i_cv_gmtime_max_time_t=32 \
	i_cv_mmap_plays_with_write=yes \
	i_cv_fd_passing=yes \
	i_cv_c99_vsnprintf=yes \
	lib_cv_va_copy=yes \
	lib_cv_va_copy=yes \
	lib_cv___va_copy=yes \
	lib_cv_va_val_copy=yes \
	ac_cv_prog_KRB5CONFIG="krb5-config"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/dovecot $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dovecot $(1)/usr/lib/
	$(call libtool_remove_files,$(1))
endef


define Package/dovecot/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/dovecot.init $(1)/etc/init.d/dovecot

	$(INSTALL_DIR) $(1)/etc/dovecot
	$(CP) $(PKG_INSTALL_DIR)/usr/share/doc/dovecot/example-config/* $(1)/etc/dovecot/

	$(INSTALL_DIR) $(1)/usr/lib/dovecot
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/dovecot/* $(1)/usr/lib/dovecot/
	rm  $(1)/usr/lib/dovecot/dovecot-config
	find $(1)/usr/lib/dovecot/ -name "*.a" -o -name "*.la" | $(XARGS) rm -f

	$(INSTALL_DIR) $(1)/usr/libexec/dovecot
	$(CP) $(PKG_INSTALL_DIR)/usr/libexec/dovecot/* $(1)/usr/libexec/dovecot/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/doveconf $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
endef

define Package/dovecot-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/doveadm $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/dsync $(1)/usr/bin/
endef

$(eval $(call BuildPackage,dovecot))
$(eval $(call BuildPackage,dovecot-utils))
