#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=postfix
PKG_VERSION:=3.5.8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= \
	https://cdn.postfix.johnriley.me/mirrors/postfix-release/official/ \
	http://ftp.porcupine.org/mirrors/postfix-release/official/
PKG_HASH:=22582628cf3edc18c5155c9ff44543dd95a9435fb68135d76a99f572cb07456f

PKG_MAINTAINER:=Denis Shulyaka <Shulyaka@gmail.com>
PKG_LICENSE:=IPL-1.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:postfix:postfix

PKG_CONFIG_DEPENDS:= \
	CONFIG_POSTFIX_TLS \
	CONFIG_POSTFIX_SASL \
	CONFIG_POSTFIX_LDAP \
	CONFIG_POSTFIX_CDB \
	CONFIG_POSTFIX_DB \
	CONFIG_POSTFIX_SQLITE \
	CONFIG_POSTFIX_MYSQL \
	CONFIG_POSTFIX_PGSQL \
	CONFIG_POSTFIX_PCRE \
	CONFIG_POSTFIX_EAI \

include $(INCLUDE_DIR)/package.mk

define Package/postfix
  SECTION:=mail
  CATEGORY:=Mail
  TITLE:=Postfix Mail Transmit Agent
  USERID:= \
  	postfix=25:postfix=25 \
  	postdrop=26:postdrop=26
  URL:=http://www.postfix.org/
  DEPENDS:=+POSTFIX_CDB:tinycdb +POSTFIX_TLS:libopenssl +POSTFIX_SASL:libsasl2 +POSTFIX_LDAP:libopenldap +POSTFIX_DB:libdb47 +POSTFIX_SQLITE:libsqlite3 +POSTFIX_MYSQL:libmysqlclient +POSTFIX_PGSQL:libpq +POSTFIX_EAI:icu +POSTFIX_PCRE:libpcre
endef

define Package/postfix/description
 Postfix is Wietse Venema's mailer that started life as an alternative to the widely-used Sendmail program. Postfix attempts to be fast, easy to administer, and secure, while at the same time being sendmail compatible enough to not upset existing users. Thus, the outside has a sendmail-ish flavor, but the inside is completely different.
endef

define Package/postfix/config
	menu "Select postfix build options"
		config POSTFIX_TLS
			bool "TLS support"
			default y
			help
			  Implements TLS support in postfix (using OpenSSL).
		config POSTFIX_SASL
			bool "SASL support"
			default y
			help
			  Implements SASL support in postfix (using Cyrus SASL).
		config POSTFIX_LDAP
			bool "LDAP support"
			default y
			help
			  Implements LDAP support in postfix (using OpenLDAP).
		config POSTFIX_DB
			bool "BerkeleyDB support"
			default n
			help
			  Implements support for btree and hash files using Berkeley DB.
		config POSTFIX_CDB
			bool "CDB support"
			default y
			help
			  Implements support for cdb files using tinycdb
		config POSTFIX_SQLITE
			bool "SQLITE support"
			default y
			help
			  Implements support for SQLite3 DB
		config POSTFIX_MYSQL
			bool "MYSQL support"
			default n
			help
			  Implements support for MySQL
		config POSTFIX_PGSQL
			bool "PostgreSQL support"
			default n
			help
			  Implement support for PostgreSQL
		config POSTFIX_PCRE
			bool "PCRE support"
			default y
			help
			  Implement support for Perl Compatible Regular Expressions
		config POSTFIX_EAI
			bool "SMTPUTF8 support"
			default n
			help
			  Enable Postfix support for Email Address Internationalization
			  (EAI) as defined in RFC 6531 (SMTPUTF8 extension), RFC 6532
			  (Internationalized email headers) and RFC 6533
			  (Internationalized delivery status notifications).
			  Since version 3.0, Postfix fully supports UTF-8 email
			  addresses and UTF-8 message header values.
	endmenu
endef

CCARGS=-DNO_NIS -fcommon
AUXLIBS=-L$(STAGING_DIR)/usr/lib

ifdef CONFIG_POSTFIX_TLS
  CCARGS+=-DUSE_TLS
  AUXLIBS+=-lssl -lcrypto
endif

ifdef CONFIG_POSTFIX_SASL
  CCARGS+=-DUSE_SASL_AUTH -DUSE_CYRUS_SASL -I$(STAGING_DIR)/usr/include/sasl
  AUXLIBS+=-lsasl2
endif

ifdef CONFIG_POSTFIX_LDAP
  CCARGS+=-DHAS_LDAP
  AUXLIBS+=-lldap -llber
endif

ifdef CONFIG_POSTFIX_CDB
  CCARGS+=-DHAS_CDB
  AUXLIBS+=-lcdb
  ifndef default_database_type
    default_database_type=cdb
  endif
endif

ifdef CONFIG_POSTFIX_DB
  AUXLIBS+=-ldb
  CCARGS+=-DHAS_DB
  ifndef default_database_type
    default_database_type=btree
  endif
else
  CCARGS+=-DNO_DB
endif

ifdef CONFIG_POSTFIX_SQLITE
  CCARGS+=-DHAS_SQLITE -I$(STAGING_DIR)/usr/include/
  AUXLIBS+=-L$(STAGING_DIR)/usr/lib -lsqlite3 -lpthread
endif

ifdef CONFIG_POSTFIX_MYSQL
  CCARGS+=-DHAS_MYSQL -I$(STAGING_DIR)/usr/include/mysql
  AUXLIBS+=-L$(STAGING_DIR)/usr/lib -lmysqlclient -lz -lm
endif

ifdef CONFIG_POSTFIX_PGSQL
  CCARGS+=-DHAS_PGSQL -I$(STAGING_DIR)/usr/include/
  AUXLIBS+=-L$(STAGING_DIR)/usr/lib -lpq
endif

ifdef CONFIG_POSTFIX_PCRE
  CCARGS+=-DHAS_PCRE -I$(STAGING_DIR)/usr/include/
  AUXLIBS+=-L$(STAGING_DIR)/usr/lib -lpcre
else
  CCARGS+=-DNO_PCRE
endif

ifdef CONFIG_POSTFIX_EAI
  AUXLIBS+=-licuuc
  CCARGS+=-DHAS_EAI
  smtputf8_conf = yes
else
  CCARGS+=-DNO_EAI
  smtputf8_conf = no
endif

CCARGS+=-DDEF_DB_TYPE=\"$(default_database_type)\"
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lresolv)

# prevent postfix-install from executing postconf at build time
# by explicitly providing the default values to postfix-install
config_directory=/etc/postfix
sample_directory=/etc/postfix
command_directory=/usr/sbin
daemon_directory=/usr/libexec/postfix
html_directory=no
manpage_directory=no
readme_directory=no
sendmail_path=/usr/sbin/sendmail
newaliases_path=/usr/bin/newaliases
mailq_path=/usr/bin/mailq
shlib_directory=/usr/lib/postfix
meta_directory=/etc/postfix
data_directory=/usr/var/lib/postfix
queue_directory=/usr/var/spool/postfix
mail_spool_directory=/usr/var/mail

ln_suffix=.postfix
ln_suffix_pre_pf=.pre-postfix
ln_suffix_legacy=.old

define Package/postfix/conffiles
$(config_directory)
endef

define Build/Configure
	if [ "$(default_database_type)" = "" ]; then \
		echo "Build error: CDB or BerkeleyDB support must be enabled."; \
		exit 1; \
	fi
	cd $(PKG_BUILD_DIR); $(MAKE) makefiles CCARGS='$(CCARGS)' $(TARGET_CONFIGURE_OPTS) AUXLIBS="$(AUXLIBS)"
endef

define Build/Compile
	# Currently postfix has a bug with Makefiles that CCARGS are not passed to the compiler, so we are copying them to CC as a workaround
	cd $(PKG_BUILD_DIR); $(MAKE) $(TARGET_CONFIGURE_OPTS) CC='$(TARGET_CC) $(CCARGS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS)'
	$(foreach p, \
		default_database_type config_directory command_directory daemon_directory \
		shlib_directory manpage_directory data_directory queue_directory \
		mail_spool_directory, \
	echo "$(p) = $($(p))" >> $(PKG_BUILD_DIR)/conf/main.cf;)
endef

define Package/postfix/install
	cd $(PKG_BUILD_DIR); $(MAKE) install_root=$(1) command_directory=$(command_directory) daemon_directory=$(daemon_directory) data_directory=$(data_directory) html_directory=$(html_directory) mail_owner=postfix mailq_path=$(mailq_path)$(ln_suffix) manpage_directory=$(manpage_directory) newaliases_path=$(newaliases_path)$(ln_suffix) queue_directory=$(queue_directory) readme_directory=$(readme_directory) sendmail_path=$(sendmail_path)$(ln_suffix) setgid_group=postdrop sample_directory=$(sample_directory) config_directory=$(config_directory) shlib_directory=$(shlib_directory) meta_directory=$(meta_directory) mail_version=$(PKG_VERSION) non-interactive-package
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/postfix.init $(1)/etc/init.d/postfix
endef

define Package/postfix/postinst
#!/bin/sh

for syml in $(sendmail_path) $(newaliases_path) $(mailq_path); do
  file=$${IPKG_INSTROOT}$${syml}
  if [ -f "$${file}$(ln_suffix_legacy)" ]; then
    # update files saved by legacy versions of Postfix postinst script
    mv "$${file}$(ln_suffix_legacy)" "$${file}$(ln_suffix_pre_pf)"
  fi
  if [ -f "$${file}" ]; then
    linktarget=$$(basename $$(readlink "$${file}"))
    postfixf=$$(basename "$${syml}")$(ln_suffix)
    if [ $${linktarget} != $${postfixf} ]; then
      mv "$${file}" "$${file}$(ln_suffix_pre_pf)"
      echo "Info: $${file} saved as $${file}$(ln_suffix_pre_pf)"
    fi
  fi
  if [ ! -f "$${file}" ]; then
    ln -s "$${syml}$(ln_suffix)" "$${file}"
  fi
done
# fix file/dir ownership in data_directory and queue_directory
if [ -z "$${IPKG_INSTROOT}" ]; then
  postfix set-permissions
fi

endef

define Package/postfix/postrm
#!/bin/sh
rm -f "$(sendmail_path)" "$(newaliases_path)" "$(mailq_path)"

for syml in "$(sendmail_path)" "$(newaliases_path)" "$(mailq_path)"; do
  file=$${IPKG_INSTROOT}$${syml}
  if [ -f "$${file}$(ln_suffix_legacy)" ]; then
    # update files saved by legacy versions of Postfix postinst script
    mv "$${file}$(ln_suffix_legacy)" "$${file}$(ln_suffix_pre_pf)"
  fi
  if [ -f "$${syml}$(ln_suffix_pre_pf)" ]; then
    mv "$${syml}$(ln_suffix_pre_pf)" "$${syml}"
    echo "Info: $${syml}$(ln_suffix_pre_pf) restored as $${syml}"
  fi
done
endef

$(eval $(call BuildPackage,postfix))
