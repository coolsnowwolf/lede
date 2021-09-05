#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openldap
PKG_VERSION:=2.4.58
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=https://gpl.savoirfairelinux.net/pub/mirrors/openldap/openldap-release/ \
	http://mirror.eu.oneandone.net/software/openldap/openldap-release/ \
	http://mirror.switch.ch/ftp/software/mirror/OpenLDAP/openldap-release/ \
	https://www.openldap.org/software/download/OpenLDAP/openldap-release/
PKG_HASH:=57b59254be15d0bf6a9ab3d514c1c05777b02123291533134a87c94468f8f47b
PKG_LICENSE:=OLDAP-2.8
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:openldap:openldap

PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS := \
        CONFIG_OPENLDAP_DEBUG \
        CONFIG_OPENLDAP_CRYPT \
        CONFIG_OPENLDAP_MONITOR \
        CONFIG_OPENLDAP_DB47 \
        CONFIG_OPENLDAP_ICU

include $(INCLUDE_DIR)/package.mk

define Package/libopenldap/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=OpenLDAP
  TITLE:=LDAP directory suite
  URL:=http://www.openldap.org/
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/libopenldap
  $(call Package/libopenldap/Default)
  MENU:=1
  DEPENDS:=+libopenssl +libsasl2 +libpthread +OPENLDAP_DB47:libdb47 +OPENLDAP_ICU:icu
  TITLE+= (libraries)
endef

define Package/libopenldap/config
  config OPENLDAP_DEBUG
	bool "Enable debugging information"
	default y
	help
		Enable debugging information. This option must be enabled
		for the loglevel directive to work.
  config OPENLDAP_CRYPT
	bool "Crypt(3) passwords support"
	default n
	help
		With crypt(3) password storage scheme enabled, OpenLDAP can
		receive and store SHA-256 and SHA-512 password hashes from
		Samba AD-DC. If this option is disabled, synchronization of
		passwords between Samba AD-DC (v4.5 and above) and OpenLDAP
		requires use of cleartext passwords.
		To enable crypt(3) password synchronization functionality:
		1. Re-include crypt(3) support in OpenWRT by enabling 'Include
		crypt() support for SHA256, SHA512 and Blowfish ciphers' option
		in "Advanced configuration options (for developers)" ->
		"Toolchain Options".
		2. Provision AD-DC with 'password hash userPassword schemes'
		option. For more information, see smb.conf manpage for details
		on 'password hash userPassword schemes'.
		3. Use a script to synchronize passwords from AD-DC to
		OpenLDAP. See samba-tool manpage for 'user syncpasswords'.
  config OPENLDAP_MONITOR
	bool "Enable monitor backend"
	default n
	help
		Enable monitor backend to obtain information about the running
		status of the daemon. See OpenLDAP documentation for more
		information.
  config OPENLDAP_DB47
	bool "Berkeley DB support"
	default n
	help
		Enable Berkeley DB support (BDB).
  config OPENLDAP_ICU
	bool "ICU support"
	default n
	help
		Enable ICU (International Components for Unicode) support.
endef

define Package/libopenldap/description
OpenLDAP Software is an open source implementation of the Lightweight Directory Access Protocol (LDAP). This package contains the shared LDAP client libraries, needed by other programs.
endef

define Package/libopenldap/conffiles
/etc/openldap/ldap.conf
endef

define Package/openldap-utils
  $(call Package/libopenldap/Default)
  DEPENDS:=+libopenldap
  TITLE+= (utilities)
endef

define Package/openldap-utils/description
This package contains client programs required to access LDAP servers.
endef

define Package/openldap-server
  $(call Package/libopenldap/Default)
  DEPENDS:=+libopenldap +libuuid
  TITLE+= (server)
endef

define Package/openldap-server/description
This package contains server programs required to provide LDAP services.
endef

define Package/openldap-server/conffiles
/etc/openldap/slapd.conf
/etc/init.d/ldap
endef

TARGET_CFLAGS += $(FPIC) -lpthread \
	-DURANDOM_DEVICE=\\\"/dev/urandom\\\"

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--enable-dynamic \
	--enable-syslog \
	--with-cyrus-sasl \
	--with-threads \
	--with-tls \
	--with-yielding_select="yes" \
	--enable-null \
	--disable-relay


ifdef CONFIG_OPENLDAP_CRYPT
	CONFIGURE_ARGS+= --enable-crypt
else
	CONFIGURE_ARGS+= --disable-crypt
endif

ifdef CONFIG_OPENLDAP_MONITOR
	CONFIGURE_ARGS+= --enable-monitor
else
	CONFIGURE_ARGS+= --disable-monitor
endif

ifdef CONFIG_OPENLDAP_DEBUG
	CONFIGURE_ARGS+= --enable-debug
else
	CONFIGURE_ARGS+= --disable-debug
endif

ifdef CONFIG_OPENLDAP_DB47
	CONFIGURE_ARGS+= \
		--enable-bdb \
		--enable-hdb
else
	CONFIGURE_ARGS+= \
		--disable-bdb \
		--disable-hdb
endif

ifndef CONFIG_OPENLDAP_ICU
	CONFIGURE_VARS += \
		ol_cv_lib_icu="no"
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		HOSTCC="$(HOSTCC)" \
		depend all install
	cd $(PKG_BUILD_DIR)/libraries/liblmdb && $(MAKE) $(CONFIGURE_VARS)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/{lber,ldap}*.h $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{lber,ldap}*.{a,so*} $(1)/usr/lib/
endef

define Package/libopenldap/install
	$(INSTALL_DIR) $(1)/etc/openldap $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/etc/openldap/ldap.conf $(1)/etc/openldap/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{lber,ldap}*.so.* $(1)/usr/lib/
endef

define Package/openldap-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ldap* $(1)/usr/bin/
endef

define Package/openldap-server/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ldap.init $(1)/etc/init.d/ldap
	$(INSTALL_DIR) $(1)/etc/openldap/schema
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/etc/openldap/schema/* $(1)/etc/openldap/schema/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/openldap/slapd.conf $(1)/etc/openldap/
	$(INSTALL_DIR) $(1)/usr/sbin
	# NB: OpenLDAP installs slapd into libexecdir, not sbindir
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/slapd $(1)/usr/sbin/
	$(eval SLAPTOOLS := slapadd slapcat slapdn slapindex slappasswd slaptest slapauth slapacl slapschema)
	for i in $(SLAPTOOLS); do \
		$(LN) ./slapd $(1)/usr/sbin/$$$$i; \
	done
endef

$(eval $(call BuildPackage,libopenldap))
$(eval $(call BuildPackage,openldap-utils))
$(eval $(call BuildPackage,openldap-server))
