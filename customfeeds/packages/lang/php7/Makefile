#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=php
PKG_VERSION:=7.4.22
PKG_RELEASE:=1

PKG_MAINTAINER:=Michael Heimpold <mhei@heimpold.de>
PKG_LICENSE:=PHP-3.01
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:php:php

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://www.php.net/distributions/
PKG_HASH:=8e078cd7d2f49ac3fcff902490a5bb1addc885e7e3b0d8dd068f42c68297bde8

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PHP7_MODULES= \
	bcmath \
	calendar ctype curl \
	dom \
	exif \
	fileinfo filter ftp \
	gettext gd gmp \
	iconv imap intl \
	json \
	ldap \
	mbstring mysqli mysqlnd \
	opcache openssl \
	pcntl pdo pdo-mysql pdo-pgsql pdo-sqlite pgsql phar \
	session shmop simplexml snmp soap sockets sqlite3 sysvmsg sysvsem sysvshm \
	tokenizer \
	xml xmlreader xmlwriter \
	zip

PKG_CONFIG_DEPENDS:= \
	$(patsubst %,CONFIG_PACKAGE_php7-mod-%,$(PHP7_MODULES)) \
	CONFIG_PHP7_LIBXML CONFIG_PHP7_SYSTEMTZDATA

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/php7/Default
  SUBMENU:=PHP7
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=PHP7 Hypertext preprocessor
  URL:=http://www.php.net/
  DEPENDS:=php7
endef

define Package/php7/Default/description
  PHP is a widely-used general-purpose scripting language that is especially
  suited for Web development and can be embedded into HTML.
endef

define Package/php7/config
	config PHP7_LIBXML
		bool "PHP7 LIBXML support"
		depends on PACKAGE_php7-cli || PACKAGE_php7-cgi

	config PHP7_SYSTEMTZDATA
		bool "Use system timezone data instead of php's built-in database"
		depends on PACKAGE_php7-cli || PACKAGE_php7-cgi
		default y
		help
			Enabling this feature automatically selects the zoneinfo-core package
			which contains data for UTC timezone. To use other timezones you have
			to install the corresponding zoneinfo-... package(s).
endef

define Package/php7
  $(call Package/php7/Default)

  DEPENDS:=+libpcre2 +zlib \
           +PHP7_LIBXML:libxml2 \
           +PHP7_SYSTEMTZDATA:zoneinfo-core
endef

define Package/php7/description
  $(call Package/php7/Default/description)
  This package contains only the PHP config file. You must actually choose
  your PHP flavour (cli, cgi or fastcgi).

  Please note, that installing php5 and php7 in parallel on the same target
  is not supported in OpenWrt/LEDE.
endef

define Package/php7-cli
  $(call Package/php7/Default)
  DEPENDS+= +PACKAGE_php7-mod-intl:libstdcpp
  TITLE+= (CLI)
endef

define Package/php7-cli/description
  $(call Package/php7/Default/description)
  This package contains the CLI version of the PHP7 interpreter.
endef

define Package/php7-cgi
  $(call Package/php7/Default)
  DEPENDS+= +PACKAGE_php7-mod-intl:libstdcpp
  TITLE+= (CGI & FastCGI)
endef

define Package/php7-cgi/description
  $(call Package/php7/Default/description)
  This package contains the CGI version of the PHP7 interpreter.
endef

define Package/php7-fastcgi
  $(call Package/php7/Default)
  DEPENDS+= +php7-cgi
  TITLE:=FastCGI startup script
endef

define Package/php7-fastcgi/description
  As FastCGI support is now a core feature the php7-fastcgi package now depends
  on the php7-cgi package, containing just the startup script.
endef

define Package/php7-fpm
  $(call Package/php7/Default)
  DEPENDS+= +PACKAGE_php7-mod-intl:libstdcpp
  TITLE+= (FPM)
endef

define Package/php7-fpm/description
  $(call Package/php7/Default/description)
  This package contains the FastCGI Process Manager of the PHP7 interpreter.
endef

define Package/php7-mod-intl/config
  config PHP7_FULLICUDATA
	bool "Add dependency to full ICU Data"
	depends on PACKAGE_php7-mod-intl
	default n
endef

define Package/php7-mod-intl/description
  Note that this package depends in ICU library which is built without data
  by default. This is to satisfy programs build and run dependencies but to
  keep the installed footprint small on the target system(s).
  However, the data is required to make the ICU library useful - and thus
  directly affects PHPs ICU extension, too - so consider to also
  select/install package 'icu-full-data'.
endef

# not everything groks --disable-nls
DISABLE_NLS:=

CONFIGURE_ARGS+= \
	--enable-cli \
	--enable-cgi \
	--enable-fpm \
	--enable-shared \
	--disable-static \
	--with-pic \
	--disable-rpath \
	--disable-debug \
	--disable-phpdbg \
	--without-pear \
	\
	--with-config-file-path=/etc \
	--with-config-file-scan-dir=/etc/php7 \
	--disable-short-tags \
	\
	--without-valgrind \
	--with-external-pcre \
	--with-zlib="$(STAGING_DIR)/usr"

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-bcmath),)
  CONFIGURE_ARGS+= --enable-bcmath=shared
else
  CONFIGURE_ARGS+= --disable-bcmath
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-calendar),)
  CONFIGURE_ARGS+= --enable-calendar=shared
else
  CONFIGURE_ARGS+= --disable-calendar
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-ctype),)
  CONFIGURE_ARGS+= --enable-ctype=shared
else
  CONFIGURE_ARGS+= --disable-ctype
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-curl),)
  CONFIGURE_ARGS+= --with-curl=shared
else
  CONFIGURE_ARGS+= --without-curl
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-dom),)
  CONFIGURE_ARGS+= --enable-dom=shared
else
  CONFIGURE_ARGS+= --disable-dom
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-exif),)
  CONFIGURE_ARGS+= --enable-exif=shared
else
  CONFIGURE_ARGS+= --disable-exif
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-fileinfo),)
  CONFIGURE_ARGS+= --enable-fileinfo=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --disable-fileinfo
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-filter),)
  CONFIGURE_ARGS+= --enable-filter=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --disable-filter
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-ftp),)
  CONFIGURE_ARGS+= --enable-ftp=shared
else
  CONFIGURE_ARGS+= --disable-ftp
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-gd),)
  CONFIGURE_ARGS+= \
	--enable-gd=shared,"$(STAGING_DIR)/usr" \
	--with-external-gd
else
  CONFIGURE_ARGS+= --disable-gd
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-gettext),)
  CONFIGURE_ARGS+= --with-gettext=shared,"$(STAGING_DIR)/usr/lib/libintl-full"
else
  CONFIGURE_ARGS+= --without-gettext
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-gmp),)
  CONFIGURE_ARGS+= --with-gmp=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-gmp
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-iconv),)
  CONFIGURE_ARGS+= --with-iconv=shared,"$(ICONV_PREFIX)"
else
  CONFIGURE_ARGS+= --without-iconv
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-imap),)
  CONFIGURE_ARGS+= \
	--with-imap=shared,"$(STAGING_DIR)/usr" \
	--with-kerberos=no \
	--with-imap-ssl="$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-imap
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-intl),)
  CONFIGURE_ARGS+= --enable-intl=shared
  TARGET_CXXFLAGS+= -std=c++0x
else
  CONFIGURE_ARGS+= --disable-intl
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-json),)
  CONFIGURE_ARGS+= --enable-json=shared
else
  CONFIGURE_ARGS+= --disable-json
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-ldap),)
  CONFIGURE_ARGS+= \
	--with-ldap=shared,"$(STAGING_DIR)/usr" \
	--with-ldap-sasl
else
  CONFIGURE_ARGS+= --without-ldap
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-mbstring),)
  CONFIGURE_ARGS+= \
	--enable-mbstring=shared \
	--enable-mbregex
else
  CONFIGURE_ARGS+= --disable-mbstring
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-mysqli),)
  CONFIGURE_ARGS+= --with-mysqli=shared
else
  CONFIGURE_ARGS+= --without-mysqli
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-mysqlnd),)
  CONFIGURE_ARGS+= --enable-mysqlnd=shared
else
  CONFIGURE_ARGS+= --disable-mysqlnd
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-opcache),)
  CONFIGURE_ARGS+= --enable-opcache=shared
else
  CONFIGURE_ARGS+= --disable-opcache
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-openssl)$(CONFIG_PACKAGE_php7-mod-ftp)$(CONFIG_PACKAGE_php7-mod-imap)$(CONFIG_PACKAGE_php7-mod-snmp),)
  CONFIGURE_ARGS+= \
	--with-openssl=shared \
	--with-kerberos=no \
	--with-openssl-dir="$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-openssl
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pcntl),)
  CONFIGURE_ARGS+= --enable-pcntl=shared
else
  CONFIGURE_ARGS+= --disable-pcntl
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pdo),)
  CONFIGURE_ARGS+= --enable-pdo=shared
  ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pdo-mysql),)
    CONFIGURE_ARGS+= --with-pdo-mysql=shared
  else
    CONFIGURE_ARGS+= --without-pdo-mysql
  endif
  ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pdo-pgsql),)
    CONFIGURE_ARGS+= --with-pdo-pgsql=shared,"$(STAGING_DIR)/usr"
  else
    CONFIGURE_ARGS+= --without-pdo-pgsql
  endif
  ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pdo-sqlite),)
    CONFIGURE_ARGS+= --with-pdo-sqlite=shared
  else
    CONFIGURE_ARGS+= --without-pdo-sqlite
  endif
else
  CONFIGURE_ARGS+= --disable-pdo
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-pgsql),)
  CONFIGURE_ARGS+= --with-pgsql=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-pgsql
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-phar),)
  CONFIGURE_ARGS+= --enable-phar=shared
else
  CONFIGURE_ARGS+= --disable-phar
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-session),)
  CONFIGURE_ARGS+= --enable-session=shared
else
  CONFIGURE_ARGS+= --disable-session
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-shmop),)
  CONFIGURE_ARGS+= --enable-shmop=shared
else
  CONFIGURE_ARGS+= --disable-shmop
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-simplexml),)
  CONFIGURE_ARGS+= --enable-simplexml=shared
else
  CONFIGURE_ARGS+= --disable-simplexml
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-snmp),)
  CONFIGURE_ARGS+= --with-snmp=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-snmp
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-soap),)
  CONFIGURE_ARGS+= --enable-soap=shared
else
  CONFIGURE_ARGS+= --disable-soap
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-sockets),)
  CONFIGURE_ARGS+= --enable-sockets=shared
else
  CONFIGURE_ARGS+= --disable-sockets
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-sqlite3),)
  CONFIGURE_ARGS+= --with-sqlite3=shared
else
  CONFIGURE_ARGS+= --without-sqlite3
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-sysvmsg),)
  CONFIGURE_ARGS+= --enable-sysvmsg=shared
else
  CONFIGURE_ARGS+= --disable-sysvmsg
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-sysvsem),)
  CONFIGURE_ARGS+= --enable-sysvsem=shared
else
  CONFIGURE_ARGS+= --disable-sysvsem
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-sysvshm),)
  CONFIGURE_ARGS+= --enable-sysvshm=shared
else
  CONFIGURE_ARGS+= --disable-sysvshm
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-tokenizer),)
  CONFIGURE_ARGS+= --enable-tokenizer=shared
else
  CONFIGURE_ARGS+= --disable-tokenizer
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-xml),)
  CONFIGURE_ARGS+= --enable-xml=shared,"$(STAGING_DIR)/usr"
  ifneq ($(CONFIG_PHP7_LIBXML),y)
    CONFIGURE_ARGS+= --with-expat
  endif
else
  CONFIGURE_ARGS+= --disable-xml
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-xmlreader),)
  CONFIGURE_ARGS+= --enable-xmlreader=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --disable-xmlreader
endif

ifneq ($(SDK)$(CONFIG_PACKAGE_php7-mod-xmlwriter),)
  CONFIGURE_ARGS+= --enable-xmlwriter=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --disable-xmlwriter
endif

ifneq ($(CONFIG_PACKAGE_php7-mod-zip),)
  CONFIGURE_ARGS+= --with-zip=shared,"$(STAGING_DIR)/usr"
else
  CONFIGURE_ARGS+= --without-zip
endif

ifneq ($(SDK)$(CONFIG_PHP7_LIBXML),)
  CONFIGURE_ARGS+= --with-libxml
else
  CONFIGURE_ARGS+= --without-libxml
endif

ifneq ($(CONFIG_PHP7_SYSTEMTZDATA),)
  CONFIGURE_ARGS+= --with-system-tzdata
else
  CONFIGURE_ARGS+= --without-system-tzdata
endif

CONFIGURE_VARS+= \
	ac_cv_c_bigendian_php=$(if $(CONFIG_BIG_ENDIAN),yes,no) \
	php_cv_cc_rpath="no" \
	iconv_impl_name="gnu_libiconv" \
	ac_cv_php_xml2_config_path="$(STAGING_DIR)/host/bin/xml2-config" \
	ac_cv_u8t_decompose=yes \
	ac_cv_have_pcre2_jit=no

define Package/php7/conffiles
/etc/php.ini
/etc/php7/
endef

define Package/php7/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/php.ini $(1)/etc/
endef

define Package/php7-cli/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/sapi/cli/php $(1)/usr/bin/php-cli
endef

define Package/php7-cgi/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/sapi/cgi/php-cgi $(1)/usr/bin/php-cgi
	ln -sf php-cgi $(1)/usr/bin/php-fcgi
endef

define Package/php7-fastcgi/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/php7-fastcgi.config $(1)/etc/config/php7-fastcgi

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/php7-fastcgi.init $(1)/etc/init.d/php7-fastcgi
endef

define Package/php7-fastcgi/conffiles
/etc/config/php7-fastcgi
endef

define Package/php7-fpm/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sapi/fpm/php-fpm $(1)/usr/bin/php-fpm

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/php7-fpm.conf $(1)/etc/php7-fpm.conf

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/php7-fpm.config $(1)/etc/config/php7-fpm

	$(INSTALL_DIR) $(1)/etc/php7-fpm.d
	$(INSTALL_DATA) ./files/php7-fpm-www.conf $(1)/etc/php7-fpm.d/www.conf

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/php7-fpm.init $(1)/etc/init.d/php7-fpm
endef

define Package/php7-fpm/conffiles
/etc/php7-fpm.conf
/etc/php7-fpm.d/
/etc/config/php7-fpm
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(if $(QUILT),,( cd $(PKG_BUILD_DIR); touch configure.ac; ./buildconf --force ))
endef

define Build/Configure
	$(if $(QUILT),( cd $(PKG_BUILD_DIR); touch configure.ac; ./buildconf --force ))
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	rm -rf $(PKG_BUILD_DIR)/staging
	make -C $(PKG_BUILD_DIR) install INSTALL_ROOT=$(PKG_BUILD_DIR)/staging
	rm -rf $(PKG_BUILD_DIR)/staging/usr/{share,man,sbin}
	rm -f $(PKG_BUILD_DIR)/staging/usr/bin/{php,php-cgi,php-cli}
	mv $(PKG_BUILD_DIR)/staging/usr/bin/phpize $(PKG_BUILD_DIR)/staging/usr/bin/phpize7
	mv $(PKG_BUILD_DIR)/staging/usr/bin/php-config $(PKG_BUILD_DIR)/staging/usr/bin/php7-config
	mv $(PKG_BUILD_DIR)/staging/usr/include/php $(PKG_BUILD_DIR)/staging/usr/include/php7
	mv $(PKG_BUILD_DIR)/staging/usr/lib/php $(PKG_BUILD_DIR)/staging/usr/lib/php7

	$(CP) $(PKG_BUILD_DIR)/staging/usr $(STAGING_DIR)/

	sed -i -e "s#prefix='/usr'#prefix='$(STAGING_DIR)/usr'#" $(STAGING_DIR)/usr/bin/phpize7
	sed -i -e "s#exec_prefix=\"\`eval echo /usr\`\"#exec_prefix='$(STAGING_DIR)/usr'#" $(STAGING_DIR)/usr/bin/phpize7
	sed -i -e "s#/include\`/php\"#/include\`/php7\"#" $(STAGING_DIR)/usr/bin/phpize7
	sed -i -e "s#/lib/php\`/build\"#/lib/php7\`/build\"#" $(STAGING_DIR)/usr/bin/phpize7

	sed -i -e "s#prefix=\"/usr\"#prefix=\"$(STAGING_DIR)/usr\"#" $(STAGING_DIR)/usr/bin/php7-config
	sed -i -e "s#/include/php\"#/include/php7\"#" $(STAGING_DIR)/usr/bin/php7-config
endef

define BuildModule

  define Package/php7-mod-$(1)
    $(call Package/php7/Default)

    ifneq ($(3),)
      DEPENDS+=$(3)
    endif

    TITLE:=$(2) shared module
  endef

  define Package/php7-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/php
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/modules/$(subst -,_,$(1)).so $$(1)/usr/lib/php/
	$(INSTALL_DIR) $$(1)/etc/php7
      ifeq ($(5),zend)
	echo "zend_extension=/usr/lib/php/$(subst -,_,$(1)).so" > $$(1)/etc/php7/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
      else
	echo "extension=$(subst -,_,$(1)).so" > $$(1)/etc/php7/$(if $(4),$(4),20)_$(subst -,_,$(1)).ini
    endif
  endef

  $$(eval $$(call BuildPackage,php7-mod-$(1)))

endef

$(eval $(call BuildPackage,php7))
$(eval $(call BuildPackage,php7-cgi))
$(eval $(call BuildPackage,php7-cli))
$(eval $(call BuildPackage,php7-fastcgi))
$(eval $(call BuildPackage,php7-fpm))

#$(eval $(call BuildModule,NAME,TITLE[,PKG DEPENDS]))
$(eval $(call BuildModule,bcmath,Bcmath))
$(eval $(call BuildModule,calendar,Calendar))
$(eval $(call BuildModule,ctype,Ctype))
$(eval $(call BuildModule,curl,cURL,+PACKAGE_php7-mod-curl:libcurl))
$(eval $(call BuildModule,dom,DOM,+@PHP7_LIBXML +PACKAGE_php7-mod-dom:libxml2))
$(eval $(call BuildModule,exif,EXIF))
$(eval $(call BuildModule,fileinfo,Fileinfo))
$(eval $(call BuildModule,filter,Filter))
$(eval $(call BuildModule,ftp,FTP,+PACKAGE_php7-mod-ftp:libopenssl))
$(eval $(call BuildModule,gd,GD graphics,+PACKAGE_php7-mod-gd:libgd-full))
$(eval $(call BuildModule,gettext,Gettext,+PACKAGE_php7-mod-gettext:libintl-full))
$(eval $(call BuildModule,gmp,GMP,+PACKAGE_php7-mod-gmp:libgmp))
$(eval $(call BuildModule,iconv,iConv,$(ICONV_DEPENDS)))
$(eval $(call BuildModule,imap,IMAP,+PACKAGE_php7-mod-imap:libopenssl +PACKAGE_libpam:libpam +PACKAGE_php7-mod-imap:uw-imap))
$(eval $(call BuildModule,intl,Internationalization Functions,+PACKAGE_php7-mod-intl:icu +PHP7_FULLICUDATA:icu-full-data))
$(eval $(call BuildModule,json,JSON))
$(eval $(call BuildModule,ldap,LDAP,+PACKAGE_php7-mod-ldap:libopenldap +PACKAGE_php7-mod-ldap:libsasl2))
$(eval $(call BuildModule,mbstring,MBString,+PACKAGE_php7-mod-mbstring:oniguruma))
$(eval $(call BuildModule,mysqli,MySQL Improved Extension,+PACKAGE_php7-mod-mysqli:php7-mod-mysqlnd,30))
$(eval $(call BuildModule,mysqlnd,MySQL Native Driver,+PACKAGE_php7-mod-openssl:php7-mod-openssl))
$(eval $(call BuildModule,opcache,OPcache,,,zend))
$(eval $(call BuildModule,openssl,OpenSSL,+PACKAGE_php7-mod-openssl:libopenssl,15))
$(eval $(call BuildModule,pcntl,PCNTL))
$(eval $(call BuildModule,pdo,PHP Data Objects))
$(eval $(call BuildModule,pdo-mysql,PDO driver for MySQL,+php7-mod-pdo +PACKAGE_php7-mod-pdo-mysql:php7-mod-mysqlnd))
$(eval $(call BuildModule,pdo-pgsql,PDO driver for PostgreSQL,+php7-mod-pdo +PACKAGE_php7-mod-pdo-pgsql:libpq))
$(eval $(call BuildModule,pdo-sqlite,PDO driver for SQLite 3.x,+php7-mod-pdo +PACKAGE_php7-mod-pdo-sqlite:libsqlite3 +PACKAGE_php7-mod-pdo-sqlite:librt))
$(eval $(call BuildModule,pgsql,PostgreSQL,+PACKAGE_php7-mod-pgsql:libpq))
$(eval $(call BuildModule,phar,Phar Archives))
$(eval $(call BuildModule,session,Session))
$(eval $(call BuildModule,shmop,Shared Memory))
$(eval $(call BuildModule,simplexml,SimpleXML,+@PHP7_LIBXML +PACKAGE_php7-mod-simplexml:libxml2))
$(eval $(call BuildModule,snmp,SNMP,+PACKAGE_php7-mod-snmp:libnetsnmp +PACKAGE_php7-mod-snmp:libopenssl))
$(eval $(call BuildModule,soap,SOAP,+@PHP7_LIBXML +PACKAGE_php7-mod-soap:libxml2))
$(eval $(call BuildModule,sockets,Sockets))
$(eval $(call BuildModule,sqlite3,SQLite3,+PACKAGE_php7-mod-sqlite3:libsqlite3))
$(eval $(call BuildModule,sysvmsg,System V messages))
$(eval $(call BuildModule,sysvsem,System V shared memory))
$(eval $(call BuildModule,sysvshm,System V semaphore))
$(eval $(call BuildModule,tokenizer,Tokenizer))
$(eval $(call BuildModule,xml,XML,+PHP7_LIBXML:libxml2 +!PHP7_LIBXML:libexpat))
$(eval $(call BuildModule,xmlreader,XMLReader,+@PHP7_LIBXML +PACKAGE_php7-mod-dom:php7-mod-dom +PACKAGE_php7-mod-xmlreader:libxml2))
$(eval $(call BuildModule,xmlwriter,XMLWriter,+@PHP7_LIBXML +PACKAGE_php7-mod-xmlwriter:libxml2))
$(eval $(call BuildModule,zip,ZIP,+PACKAGE_php7-mod-zip:libzip))
