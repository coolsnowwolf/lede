#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=krb5
PKG_VERSION:=1.19.1
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=NOTICE
PKG_CPE_ID:=cpe:/a:mit:kerberos

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://web.mit.edu/kerberos/dist/krb5/1.19
PKG_HASH:=fa16f87eb7e3ec3586143c800d7eaff98b5e0dcdf0772af7d98612e49dbeb20b

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=src

define Package/krb5/Default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Kerberos
	URL:=https://web.mit.edu/kerberos/
endef

define Package/krb5-libs
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Kerberos
	DEPENDS:=+libncurses +libss +libcomerr
	TITLE:=Kerberos 5 Shared Libraries
	URL:=http://web.mit.edu/kerberos/
endef

define Package/krb5-server
	$(call Package/krb5/Default)
	DEPENDS:=+krb5-libs +libpthread
	TITLE:=Kerberos 5 Server
endef

define Package/krb5-server-extras
	$(call Package/krb5/Default)
	DEPENDS:=+krb5-libs +libpthread
	TITLE:=Kerberos 5 Admin Server
endef

define Package/krb5-client
	$(call Package/krb5/Default)
	DEPENDS:=+krb5-libs
	TITLE:=Kerberos 5 Client
endef

define Package/krb5/description
  Kerberos is a network authentication protocol. 
  It is designed to provide strong authentication for client/server applications by using secret-key cryptography.
endef

CONFIGURE_PATH = ./src

CONFIGURE_VARS += \
	cross_compiling=yes \
	krb5_cv_attr_constructor_destructor=yes,yes \
	krb5_cv_sys_rcdir=/tmp \
	ac_cv_func_regcomp=yes \
	ac_cv_printf_positional=yes \
	ac_cv_file__etc_environment=no \
	ac_cv_file__etc_TIMEZONE=no \
	ac_cv_header_keyutils_h=no

CONFIGURE_ARGS += \
	--localstatedir=/etc \
	--with-system-ss \
	--with-system-et \
	--without-system-verto \
	--without-tcl \
	--without-tls-impl \
	--without-libedit \
	--without-readline \
	--disable-rpath \
	--disable-pkinit \
	--with-size-optimizations \
	--with-crypto-impl=builtin

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include $(1)/usr
	$(INSTALL_DIR) $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/lib $(1)/usr
	# needed for samba4, to detect system-krb5
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/krb5-config $(1)/usr/bin
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' $(1)/usr/bin/krb5-config
	$(INSTALL_DIR) $(2)/bin
	$(LN) ../../usr/bin/krb5-config $(2)/bin/krb5-config
endef

define Package/krb5-libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/krb5
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/kdb
	$(INSTALL_DIR) $(1)/usr/lib/krb5/plugins/libkrb5
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/krb5/plugins/kdb/db2.so $(1)/usr/lib/krb5/plugins/kdb
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

define Package/krb5-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdestroy $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kinit $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/klist $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kpasswd $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ksu $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kvno $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/krb5-send-pr $(1)/usr/sbin
endef

# Removed some server-side software to reduce package size. This should be
# put in a separate package if needed.
define Package/krb5-server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/krb5kdc $(1)/etc/init.d/krb5kdc
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmin.local $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kdb5_util $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/krb5kdc $(1)/usr/sbin
endef

define Package/krb5-server-extras/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kadmind $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kprop $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kpropd $(1)/usr/sbin
endef

define Package/krb5-server/postinst
mkdir -p $${IPKG_INSTROOT}/etc/krb5kdc
touch $${IPKG_INSTROOT}/etc/krb5kdc/kadm5.acl
endef

$(eval $(call BuildPackage,krb5-libs))
$(eval $(call BuildPackage,krb5-server))
$(eval $(call BuildPackage,krb5-server-extras))
$(eval $(call BuildPackage,krb5-client))
