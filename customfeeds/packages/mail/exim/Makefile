include $(TOPDIR)/rules.mk

PKG_NAME:=exim
PKG_VERSION:=4.94.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://ftp.exim.org/pub/exim/exim4/
PKG_HASH:=051861fc89f06205162f12129fb7ebfe473383bb6194bf8642952bfd50329274
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=LICENSE NOTICE
PKG_CPE_ID:=cpe:/a:exim:exim

PKG_CONFIG_DEPENDS:=\
	CONFIG_BUILD_NLS \
	CONFIG_PACKAGE_exim \
	CONFIG_PACKAGE_exim-gnutls \
	CONFIG_EXIM_GNUTLS_DANE \
	CONFIG_PACKAGE_exim-openssl \
	CONFIG_PACKAGE_exim-ldap \
	CONFIG_PACKAGE_exim-lookup-mysql \
	CONFIG_PACKAGE_exim-lookup-pgsql \
	CONFIG_PACKAGE_exim-lookup-redis \
	CONFIG_PACKAGE_exim-lookup-sqlite

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/nls.mk

LOOKUPS:= \
	cdb \
	dbmdb \
	dnsdb \
	json%+PACKAGE_exim-lookup-json:jansson \
	mysql%+PACKAGE_exim-lookup-mysql:libmariadb \
	passwd \
	pgsql%+PACKAGE_exim-lookup-pgsql:libpq \
	redis%+PACKAGE_exim-lookup-redis:libhiredis \
	sqlite%+PACKAGE_exim-lookup-sqlite:libsqlite3

define Package/exim/Default
  SECTION:=mail
  CATEGORY:=Mail
  DEPENDS:=+libdb47 +libpcre $(ICONV_DEPENDS) +BUILD_NLS:libidn2 +BUILD_NLS:libidn
  TITLE:=Exim message transfer agent
  URL:=http://www.exim.org/
  USERID:=mail=42:mail=42
endef

define Package/exim
  $(call Package/exim/Default)
  VARIANT:=nossl
  CONFLICTS:=exim-openssl exim-gnutls exim-ldap
endef

define Package/exim-gnutls
  $(call Package/exim/Default)
  TITLE+=(with GnuTLS)
  VARIANT:=gnutls
  DEPENDS+=+PACKAGE_exim-gnutls:libgnutls +EXIM_GNUTLS_DANE:libgnutls-dane
  PROVIDES:=exim
  CONFLICTS:=exim-openssl exim-ldap
endef
define Package/exim-openssl
  $(call Package/exim/Default)
  TITLE+=(with OpenSSL)
  VARIANT:=openssl
  DEPENDS+=+PACKAGE_exim-openssl:libopenssl
  PROVIDES:=exim
  CONFLICTS:=exim-ldap
endef

define Package/exim-ldap
  $(call Package/exim/Default)
  TITLE+=(with OpenSSL and OpenLDAP)
  VARIANT:=ldap
  DEPENDS+=+PACKAGE_exim-ldap:libopenssl +PACKAGE_exim-ldap:libsasl2 +PACKAGE_exim-ldap:libopenldap
  PROVIDES:=exim
endef

define Package/exim/Default/description
Exim is a message transfer agent (MTA) developed at the University of
Cambridge for use on Unix systems connected to the Internet.
endef

define Package/exim/description
$(call Package/exim/Default/description)

This package provides Exim without TLS support.
endef

define Package/exim-gnutls/description
$(call Package/exim/Default/description)

This package provides Exim built with GnuTLS.
endef

define Package/exim-gnutls/config
  	config EXIM_GNUTLS_DANE
  		bool "exim-gnutls DANE support"
  			depends on PACKAGE_exim-gnutls
  			default n
  			help
  				Build exim-gnutls against libgnutls-dane for DANE support.
  				libgnutls-dane depends on libunbound which depends on libopenssl.
endef

define Package/exim-openssl/description
$(call Package/exim/Default/description)

This package provides Exim built with OpenSSL.
endef

define Package/exim-ldap/description
$(call Package/exim/Default/description)

This package provides Exim built with OpenSSL, OpenLDAP and Cyrus SASL.
endef

define LookupGen
define Package/exim-lookup-$(subst _,-,$(firstword $(subst %, ,$(1))))
  SECTION:=mail
  CATEGORY:=Mail
  TITLE:=Exim lookup module $(firstword $(subst %, ,$(1)))
  URL:=http://www.exim.org/
  DEPENDS:=exim $(wordlist 2,$(words $(subst %, ,$(1))),$(subst %, ,$(1)))
endef
endef

$(foreach file,$(LOOKUPS),$(eval $(call LookupGen,$(file))))

define Package/exim/conffiles
/etc/exim/exim.conf
endef

HOST_CFLAGS += -std=c99
TARGET_CFLAGS += $(FPIC) -DNO_IP_OPTIONS -D_FILE_OFFSET_BITS=64

MAKE_VARS += build=Linux-$$(ARCH)
MAKE_FLAGS += AR="$$(TARGET_AR) r"
HOST_MAKE_VARS += build=Linux-$$(ARCH)

define Build/Configure
	$(CP) $(PKG_BUILD_DIR)/src/EDITME $(PKG_BUILD_DIR)/Local/Makefile
	echo "PID_FILE_PATH=/var/run/exim.pid" >> $(PKG_BUILD_DIR)/Local/Makefile
	echo "BIN_DIRECTORY=/usr/sbin" >> $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%CONFIGURE_FILE=/usr/exim/configure%CONFIGURE_FILE=/etc/exim/exim.conf%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# EXIM_GROUP=%EXIM_GROUP=42%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# EXIM_USER=exim%EXIM_USER=42%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# INCLUDE=.*%INCLUDE=-I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# HAVE_IPV6=YES%HAVE_IPV6=YES%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# WITH_CONTENT_SCAN=yes%WITH_CONTENT_SCAN=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# SUPPORT_MAILDIR=yes%SUPPORT_MAILDIR=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# SUPPORT_MAILSTORE=yes%SUPPORT_MAILSTORE=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# SUPPORT_MBX=yes%SUPPORT_MBX=yes%' $(PKG_BUILD_DIR)/Local/Makefile
# enable lookup modules
	$(SED) 's%# LOOKUP_DSEARCH=yes%LOOKUP_DSEARCH=yes%' $(PKG_BUILD_DIR)/Local/Makefile
ifdef CONFIG_PACKAGE_exim-lookup-dbmdb
	$(SED) 's%LOOKUP_DBM=yes%LOOKUP_DBM=2%' $(PKG_BUILD_DIR)/Local/Makefile
else
	$(SED) 's%LOOKUP_DBM=yes%# LOOKUP_DBM=yes%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-dnsdb
	$(SED) 's%LOOKUP_DNSDB=yes%LOOKUP_DNSDB=2%' $(PKG_BUILD_DIR)/Local/Makefile
else
	$(SED) 's%LOOKUP_DNSDB=yes%# LOOKUP_DNSDB=yes%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-passwd
	$(SED) 's%# LOOKUP_PASSWD=yes%LOOKUP_PASSWD=2%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-cdb
	$(SED) 's%# LOOKUP_CDB=yes%LOOKUP_CDB=2%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-json
	$(SED) 's%# LOOKUP_JSON=yes%LOOKUP_JSON=2\
		\nLOOKUP_JSON_INCLUDE=-I$(STAGING_DIR)/usr/include\
		\nLOOKUP_JSON_LIBS=-Wl,--no-as-needed -ljansson%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-pgsql
	$(SED) 's%# LOOKUP_PGSQL=yes%LOOKUP_PGSQL=2\
		\nLOOKUP_PGSQL_LIBS=-Wl,--no-as-needed -lpq%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-sqlite
	$(SED) 's%# LOOKUP_SQLITE=yes%LOOKUP_SQLITE=2\
		\nLOOKUP_SQLITE_LIBS=-Wl,--no-as-needed -lsqlite3%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-mysql
	$(SED) 's%# LOOKUP_MYSQL=yes%LOOKUP_MYSQL=2\
		\nLOOKUP_MYSQL_INCLUDE=-I$(STAGING_DIR)/usr/include/mysql\
		\nLOOKUP_MYSQL_LIBS=-Wl,--no-as-needed -lmysqlclient%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifdef CONFIG_PACKAGE_exim-lookup-redis
	$(SED) 's%# LOOKUP_REDIS=yes%LOOKUP_REDIS=2\
		\nLOOKUP_REDIS_INCLUDE=-I$(STAGING_DIR)/usr/include/hiredis\
		\nLOOKUP_REDIS_LIBS=-Wl,--no-as-needed -lhiredis%' $(PKG_BUILD_DIR)/Local/Makefile
endif

ifeq ($(CONFIG_BUILD_NLS),y)
	$(SED) 's%# HAVE_ICONV=yes%HAVE_ICONV=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# CFLAGS=-O -I/usr/local/include%CFLAGS=$(TARGET_CFLAGS) $(ICONV_CPPFLAGS)%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# EXTRALIBS_EXIM=-L/usr/local/lib -liconv%EXTRALIBS_EXIM=-export-dynamic -rdynamic $(ICONV_LDFLAGS) -liconv -ldl%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# SUPPORT_I18N_2008=yes%SUPPORT_I18N_2008=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# LDFLAGS += -lidn -lidn2%LDFLAGS += -lidn -lidn2%' $(PKG_BUILD_DIR)/Local/Makefile
else
	$(SED) 's%# HAVE_ICONV=yes%HAVE_ICONV=no%' $(PKG_BUILD_DIR)/Local/Makefile
endif

	$(SED) 's%# AUTH_CRAM_MD5=yes%AUTH_CRAM_MD5=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_DOVECOT=yes%AUTH_DOVECOT=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_EXTERNAL=yes%AUTH_EXTERNAL=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_PLAINTEXT=yes%AUTH_PLAINTEXT=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_SPA=yes%AUTH_SPA=yes%' $(PKG_BUILD_DIR)/Local/Makefile

ifeq ($(BUILD_VARIANT),gnutls)
	$(SED) 's%# USE_GNUTLS=yes%USE_GNUTLS=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# TLS_INCLUDE=-I/usr/local/.*%TLS_INCLUDE=-I$(STAGING_DIR)/usr/include%' $(PKG_BUILD_DIR)/Local/Makefile
ifeq ($(CONFIG_EXIM_GNUTLS_DANE),y)
	$(SED) 's%# TLS_LIBS=-lgnutls -lgnutls-dane%TLS_LIBS=-L$(STAGING_DIR)/usr/lib -lgnutls -lgnutls-dane%' $(PKG_BUILD_DIR)/Local/Makefile
else
	$(SED) 's%SUPPORT_DANE=yes%# SUPPORT_DANE=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# TLS_LIBS=-lgnutls -lgnutls-dane%TLS_LIBS=-L$(STAGING_DIR)/usr/lib -lgnutls%' $(PKG_BUILD_DIR)/Local/Makefile
endif
	$(SED) 's%# AUTH_TLS=yes%AUTH_TLS=yes%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifneq ($(filter ldap openssl, $(BUILD_VARIANT)),)
	$(SED) 's%# USE_OPENSSL=yes%USE_OPENSSL=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# TLS_INCLUDE=-I/usr/local/.*%TLS_INCLUDE=-I$(STAGING_DIR)/usr/include%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# TLS_LIBS=-lssl -lcrypto%TLS_LIBS=-L$(STAGING_DIR)/usr/lib -lssl -lcrypto%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_TLS=yes%AUTH_TLS=yes%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifeq ($(BUILD_VARIANT),ldap)
	$(SED) 's%# LOOKUP_LDAP=yes%LOOKUP_LDAP=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# LDAP_LIB_TYPE=OPENLDAP2%LDAP_LIB_TYPE=OPENLDAP2%' $(PKG_BUILD_DIR)/Local/Makefile
	echo "LOOKUP_LIBS+=-lldap -llber" >> $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_CYRUS_SASL=yes%AUTH_CYRUS_SASL=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# AUTH_LIBS=-lsasl2%AUTH_LIBS=-lsasl2%' $(PKG_BUILD_DIR)/Local/Makefile
endif
ifeq ($(BUILD_VARIANT),nossl)
	$(SED) 's%# DISABLE_TLS=yes%DISABLE_TLS=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%SUPPORT_DANE=yes%# SUPPORT_DANE=yes%' $(PKG_BUILD_DIR)/Local/Makefile
endif
	$(SED) 's%# CFLAGS_DYNAMIC=-shared -rdynamic -fPIC%CFLAGS_DYNAMIC=-shared -rdynamic $(FPIC)%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%# LOOKUP_MODULE_DIR=/usr/lib/exim/lookups/%LOOKUP_MODULE_DIR=/usr/lib/exim/lookups/%' $(PKG_BUILD_DIR)/Local/Makefile
	$(SED) 's%#DLOPEN_LOCAL_SCAN=yes%DLOPEN_LOCAL_SCAN=yes%' $(PKG_BUILD_DIR)/Local/Makefile
	$(call Build/Compile/Default,makefile)
	$(CP) $(PKG_BUILD_DIR)/OS/os.h-Linux $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	# overwrite types for cross-compile
	# (is all the below true for glibc as well?)
	echo '#include <inttypes.h>' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#define ip_options ip_opts' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#undef OFF_T_FMT' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#define OFF_T_FMT "%" PRId64' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#undef LONGLONG_T' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#define LONGLONG_T int64_t' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#ifndef NS_MAXMSG' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#define NS_MAXMSG 65535' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	echo '#endif' >> $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/os.h
	$(HOST_MAKE_VARS) $(MAKE) $(HOST_MAKE_FLAGS) $(HOST_MAKE_VARS) -C $(PKG_BUILD_DIR)/build-Linux-$(ARCH) macro_predef
endef

define Package/exim/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build-Linux-$(ARCH)/exim $(1)/usr/sbin
endef

Package/exim-gnutls/install = $(Package/exim/install)
Package/exim-openssl/install = $(Package/exim/install)
Package/exim-ldap/install = $(Package/exim/install)

define LookupInstall
define Package/exim-lookup-$(subst _,-,$(firstword $(subst %, ,$(1))))/install
	$(INSTALL_DIR) $$(1)/usr/lib/exim/lookups
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/build-Linux-$(ARCH)/lookups/$(firstword $(subst %, ,$(1))).so \
		$$(1)/usr/lib/exim/lookups
endef
endef

$(foreach file,$(LOOKUPS),$(eval $(call LookupInstall,$(file))))

$(eval $(call BuildPackage,exim))
$(eval $(call BuildPackage,exim-gnutls))
$(eval $(call BuildPackage,exim-openssl))
$(eval $(call BuildPackage,exim-ldap))
$(foreach file,$(LOOKUPS),$(eval $(call BuildPackage,exim-lookup-$(subst _,-,$(firstword $(subst %, ,$(file)))))))
