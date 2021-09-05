#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lighttpd
PKG_VERSION:=1.4.59
PKG_RELEASE:=2
# release candidate ~rcX testing; remove for release
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-1.4.59

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://download.lighttpd.net/lighttpd/releases-1.4.x
PKG_HASH:=fb953db273daef08edb6e202556cae8a3d07eed6081c96bd9903db957d1084d5

PKG_MAINTAINER:=W. Michael Petullo <mike@flyn.org>
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:lighttpd:lighttpd

PKG_INSTALL:=1
PKG_BUILD_DEPENDS:=meson/host
PKG_CONFIG_DEPENDS:=CONFIG_LIGHTTPD_SSL $(patsubst %,CONFIG_PACKAGE_lighttpd-mod-%,$(REBUILD_MODULES))

REBUILD_MODULES=authn_gssapi authn_ldap authn_mysql cml magnet mysql_vhost trigger_b4_dl webdav

include $(INCLUDE_DIR)/package.mk
include ../../devel/meson/meson.mk

define Package/lighttpd/Default
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  URL:=https://www.lighttpd.net/
endef

define Package/lighttpd
  $(call Package/lighttpd/Default)
  MENU:=1
  DEPENDS:=+libnettle +libpcre +libpthread +LIGHTTPD_LOGROTATE:logrotate
  TITLE:=A flexible and lightweight web server
endef

define Package/lighttpd/config
config LIGHTTPD_SSL
	bool "SSL support"
	depends on PACKAGE_lighttpd
	default y
	help
	  Implements SSL support in lighttpd (using libopenssl). This
	  option is enabled by default for backwards compatibility.
          Select one TLS module below if you enable the SSL engine in
	  your lighttpd configuration file.
          (mod_gnutls, mod_mbedtls, mod_nss, mod_openssl, mod_wolfssl)

config LIGHTTPD_SSL_DEPENDS
        bool
        depends on LIGHTTPD_SSL
        default PACKAGE_lighttpd-mod-mbedtls || PACKAGE_lighttpd-mod-wolfssl || PACKAGE_lighttpd-mod-gnutls || PACKAGE_lighttpd-mod-nss

config LIGHTTPD_SSL_SELECT
        tristate
        depends on LIGHTTPD_SSL
        default m if !LIGHTTPD_SSL_DEPENDS
        select PACKAGE_lighttpd-mod-openssl

config LIGHTTPD_LOGROTATE
	bool "Logrotate support"
	depends on PACKAGE_lighttpd
	default n
	help
	  It adds support for logrotate functionality.
endef

MESON_ARGS += \
	-Dwith_brotli=false \
	-Dwith_bzip=false \
	-Dwith_dbi=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_dbi)$(CONFIG_PACKAGE_lighttpd-mod-vhostdb_dbi),true,false) \
	-Dwith_fam=false \
	-Dwith_gdbm=$(if $(CONFIG_PACKAGE_lighttpd-mod-trigger_b4_dl),true,false) \
	-Dwith_geoip=false \
	-Dwith_gnutls=$(if $(CONFIG_PACKAGE_lighttpd-mod-gnutls),true,false) \
	-Dwith_krb5=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_gssapi),true,false) \
	-Dwith_ldap=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_ldap)$(CONFIG_PACKAGE_lighttpd-mod-vhostdb_ldap),true,false) \
	-Dwith_libev=false \
	-Dwith_libunwind=false \
	-Dwith_lua=$(if $(CONFIG_PACKAGE_lighttpd-mod-cml)$(CONFIG_PACKAGE_lighttpd-mod-magnet),true,false) \
	-Dwith_maxminddb=$(if $(CONFIG_PACKAGE_lighttpd-mod-maxminddb),true,false) \
	-Dwith_mbedtls=$(if $(CONFIG_PACKAGE_lighttpd-mod-mbedtls),true,false) \
	-Dwith_memcached=false \
	-Dwith_mysql=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_mysql)$(CONFIG_PACKAGE_lighttpd-mod-mysql_vhost)$(CONFIG_PACKAGE_lighttpd-mod-vhostdb_mysql),true,false) \
	-Dwith_nettle=true \
	-Dwith_nss=$(if $(CONFIG_PACKAGE_lighttpd-mod-nss),true,false) \
	-Dwith_openssl=$(if $(CONFIG_PACKAGE_lighttpd-mod-openssl),true,false) \
	-Dwith_pam=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_pam),true,false) \
	-Dwith_pcre=true \
	-Dwith_pgsql=$(if $(CONFIG_PACKAGE_lighttpd-mod-vhostdb_pgsql),true,false) \
	-Dwith_sasl=$(if $(CONFIG_PACKAGE_lighttpd-mod-authn_sasl),true,false) \
	-Dwith_webdav_locks=$(if $(CONFIG_PACKAGE_lighttpd-mod-webdav),true,false) \
	-Dwith_webdav_props=$(if $(CONFIG_PACKAGE_lighttpd-mod-webdav),true,false) \
	-Dwith_wolfssl=$(if $(CONFIG_PACKAGE_lighttpd-mod-wolfssl),true,false) \
	-Dwith_xattr=false \
	-Dwith_zlib=$(if $(CONFIG_PACKAGE_lighttpd-mod-deflate),true,false) \
	-Dwith_zstd=false

BASE_MODULES:=dirlisting indexfile staticfile

define Package/lighttpd/conffiles
/etc/lighttpd/lighttpd.conf
endef

define Package/lighttpd/install
	$(INSTALL_DIR) $(1)/etc/lighttpd
	$(INSTALL_DATA) ./files/lighttpd.conf $(1)/etc/lighttpd/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/doc/config/conf.d/mime.conf $(1)/etc/lighttpd/
	$(INSTALL_DIR) $(1)/etc/lighttpd/conf.d
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/lighttpd.init $(1)/etc/init.d/lighttpd
	$(INSTALL_DIR) $(1)/usr/lib/lighttpd
	for m in $(BASE_MODULES); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/lighttpd/mod_$$$${m}.so $(1)/usr/lib/lighttpd/ ; \
	done
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/lighttpd $(1)/usr/sbin/

ifneq ($(strip $(CONFIG_LIGHTTPD_LOGROTATE)),)
	$(INSTALL_DIR) $(1)/etc/logrotate.d
	$(CP) ./files/lighttpd.logrotate $(1)/etc/logrotate.d/lighttpd.conf
endif
endef

define BuildPlugin
  define Package/lighttpd-mod-$(1)
    $(call Package/lighttpd/Default)
    DEPENDS:=lighttpd
    ifneq ($(3),)
      DEPENDS+= $(3)
    endif
    TITLE:=$(2) module
  endef

  define Package/lighttpd-mod-$(1)/conffiles
/etc/lighttpd/conf.d/$(4)-$(1).conf
  endef

 ifneq ($(SDK)$(CONFIG_PACKAGE_lighttpd-mod-$(1)),)
  define Package/lighttpd-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/lighttpd
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lighttpd/mod_$(1).so $$(1)/usr/lib/lighttpd
	$(INSTALL_DIR) $$(1)/etc/lighttpd/conf.d
	if [ -f $(PKG_BUILD_DIR)/doc/config/conf.d/$(1).conf ]; then \
		$(CP) $(PKG_BUILD_DIR)/doc/config/conf.d/$(1).conf $$(1)/etc/lighttpd/conf.d/$(4)-$(1).conf ; \
		if ! grep -qF 'mod_$(1)' $$(1)/etc/lighttpd/conf.d/$(4)-$(1).conf; then \
			sed -i "`sed '/^##/ !q' $$(1)/etc/lighttpd/conf.d/$(4)-$(1).conf | wc -l` i\
server.modules += ( \"mod_$(1)\" )" $$(1)/etc/lighttpd/conf.d/$(4)-$(1).conf ; \
		fi \
	else \
		echo 'server.modules += ( "mod_$(1)" )' > $$(1)/etc/lighttpd/conf.d/$(4)-$(1).conf ; \
	fi
  endef
 endif

  $$(eval $$(call BuildPackage,lighttpd-mod-$(1)))
endef

$(eval $(call BuildPackage,lighttpd))

# First, permit redirect from HTTP to HTTPS.
$(eval $(call BuildPlugin,redirect,URL redirection,+PACKAGE_lighttpd-mod-redirect:libpcre,10))

# Next, permit authentication.
$(eval $(call BuildPlugin,auth,Authentication,+PACKAGE_lighttpd-mod-auth:libnettle,20))
$(eval $(call BuildPlugin,authn_dbi,DBI-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_dbi:libnettle +PACKAGE_lighttpd-mod-authn_dbi:libdbi,20))
$(eval $(call BuildPlugin,authn_file,File-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_file:libnettle,20))
$(eval $(call BuildPlugin,authn_gssapi,Kerberos-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_gssapi:krb5-libs,20))
$(eval $(call BuildPlugin,authn_ldap,LDAP-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_ldap:libopenldap,20))
$(eval $(call BuildPlugin,authn_mysql,Mysql-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_mysql:libnettle +PACKAGE_lighttpd-mod-authn_mysql:libmariadb,20))
$(eval $(call BuildPlugin,authn_pam,PAM-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_pam:libpam,20))
$(eval $(call BuildPlugin,authn_sasl,SASL-based authentication,lighttpd-mod-auth +PACKAGE_lighttpd-mod-authn_sasl:libsasl2,20))

# Finally, everything else.
$(eval $(call BuildPlugin,access,Access restrictions,,30))
$(eval $(call BuildPlugin,accesslog,Access logging,,30))
$(eval $(call BuildPlugin,ajp13,AJP13 Tomcat connector,,30))
$(eval $(call BuildPlugin,alias,Directory alias,,30))
$(eval $(call BuildPlugin,cgi,CGI,,30))
$(eval $(call BuildPlugin,cml,Cache Meta Language,+PACKAGE_lighttpd-mod-cml:liblua +PACKAGE_lighttpd-mod-cml:libnettle,30))
$(eval $(call BuildPlugin,deflate,Compress dynamic output,+PACKAGE_lighttpd-mod-deflate:zlib,30))
$(eval $(call BuildPlugin,evasive,Evasive,,30))
$(eval $(call BuildPlugin,evhost,Enhanced Virtual-Hosting,,30))
$(eval $(call BuildPlugin,expire,Expire,,30))
$(eval $(call BuildPlugin,extforward,Extract client,,30))
$(eval $(call BuildPlugin,fastcgi,FastCGI,,30))
$(eval $(call BuildPlugin,flv_streaming,FLV streaming,,30))
$(eval $(call BuildPlugin,gnutls,TLS using gnutls,@LIGHTTPD_SSL +PACKAGE_lighttpd-mod-gnutls:libgnutls,30))
$(eval $(call BuildPlugin,magnet,Magnet,+PACKAGE_lighttpd-mod-magnet:liblua,30))
$(eval $(call BuildPlugin,maxminddb,MaxMind DB,+PACKAGE_lighttpd-mod-maxminddb:libmaxminddb,30))
$(eval $(call BuildPlugin,mbedtls,TLS using mbedtls,@LIGHTTPD_SSL +PACKAGE_lighttpd-mod-mbedtls:libmbedtls,30))
$(eval $(call BuildPlugin,nss,TLS using nss,@LIGHTTPD_SSL +PACKAGE_lighttpd-mod-nss:libnss,30))
$(eval $(call BuildPlugin,openssl,TLS using openssl,@LIGHTTPD_SSL +PACKAGE_lighttpd-mod-openssl:libopenssl,30))
$(eval $(call BuildPlugin,proxy,Proxy,,30))
$(eval $(call BuildPlugin,rewrite,URL rewriting,+PACKAGE_lighttpd-mod-rewrite:libpcre,30))
$(eval $(call BuildPlugin,rrdtool,RRDtool,,30))
$(eval $(call BuildPlugin,scgi,SCGI,,30))
$(eval $(call BuildPlugin,secdownload,Secure and fast download,+PACKAGE_lighttpd-mod-secdownload:libnettle,30))
$(eval $(call BuildPlugin,setenv,Environment variable setting,,30))
$(eval $(call BuildPlugin,simple_vhost,Simple virtual hosting,,30))
$(eval $(call BuildPlugin,sockproxy,sockproxy,,30))
$(eval $(call BuildPlugin,ssi,SSI,+PACKAGE_lighttpd-mod-ssi:libpcre,30))
$(eval $(call BuildPlugin,staticfile,staticfile,,30))
$(eval $(call BuildPlugin,status,Server status display,,30))
$(eval $(call BuildPlugin,trigger_b4_dl,Trigger before download,+PACKAGE_lighttpd-mod-trigger_b4_dl:libpcre +PACKAGE_lighttpd-mod-trigger_b4_dl:libgdbm,30))
$(eval $(call BuildPlugin,uploadprogress,Upload Progress,,30))
$(eval $(call BuildPlugin,userdir,User directory,,30))
$(eval $(call BuildPlugin,usertrack,User tracking,+PACKAGE_lighttpd-mod-usertrack:libnettle,30))
$(eval $(call BuildPlugin,vhostdb,Virtual Host Database,,30))
$(eval $(call BuildPlugin,vhostdb_dbi,Virtual Host Database (DBI),lighttpd-mod-vhostdb +PACKAGE_lighttpd-mod-vhostdb_dbi:libdbi,30))
$(eval $(call BuildPlugin,vhostdb_ldap,Virtual Host Database (LDAP),lighttpd-mod-vhostdb +PACKAGE_lighttpd-mod-vhostdb_ldap:libopenldap,30))
$(eval $(call BuildPlugin,vhostdb_mysql,Virtual Host Database (MariaDB),lighttpd-mod-vhostdb +PACKAGE_lighttpd-mod-vhostdb_mysql:libmariadb,30))
$(eval $(call BuildPlugin,vhostdb_pgsql,Virtual Host Database (PostgreSQL),lighttpd-mod-vhostdb +PACKAGE_lighttpd-mod-vhostdb_pgsql:libpq,30))
$(eval $(call BuildPlugin,mysql_vhost,Mysql virtual hosting,lighttpd-mod-vhostdb +PACKAGE_lighttpd-mod-mysql_vhost:libmariadb,30))
$(eval $(call BuildPlugin,webdav,WebDAV,+PACKAGE_lighttpd-mod-webdav:libsqlite3 +PACKAGE_lighttpd-mod-webdav:libuuid +PACKAGE_lighttpd-mod-webdav:libxml2,30))
$(eval $(call BuildPlugin,wolfssl,TLS using wolfssl,@LIGHTTPD_SSL +PACKAGE_lighttpd-mod-wolfssl:libwolfssl,30))
$(eval $(call BuildPlugin,wstunnel,Websocket tunneling,+PACKAGE_lighttpd-mod-wstunnel:libnettle,30))
